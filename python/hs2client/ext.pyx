# Copyright 2016 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# distutils: language = c++
# cython: embedsignature = True
# cython: wraparound = False
# cython: boundscheck = False

# Cython interface modeling the HiveServer2 concepts exposed in the C++
# hs2client library in a fairly 1-1 way without any details of the Python DB
# API 2.0

from hs2client.libhs2client cimport *
from hs2client.compat import frombytes, tobytes, Decimal

cimport numpy as cnp
cnp.import_array()

import time

class HS2Exception(Exception):
    pass

class NativeException(HS2Exception):
    pass


# These functions are responsible for their own GIL acquisition internally if
# needed.
cdef extern from "converters.h" namespace "hs2client::py" nogil:
    object ConvertColumnPandas(const vector[CColumnarRowSet*]& batches,
                               int i, const CColumnType* type)



cdef check_status(const Status& status):
    if status.ok():
        return

    cdef string c_message = status.GetMessage()
    raise NativeException('C++ error: {0!s}'.format(frombytes(c_message)))


cdef class Service:
    """
    Interface to a HiveServer2 service
    """
    cdef:
        unique_ptr[CService] service
        string host
        int port
        string user
        ProtocolVersion proto_version
        int timeout_ms

    def __cinit__(self, host, port, user, protocol='v7', timeout_ms=0):
        protocol = protocol.upper()
        if protocol == 'V7':
            self.proto_version = Protocol_V7
        elif protocol == 'V6':
            self.proto_version = Protocol_V6
        else:
            raise NotImplementedError(protocol)

        self.host = tobytes(host)
        self.port = port
        self.user = tobytes(user)
        self.timeout_ms = timeout_ms

        check_status(CService.Connect(self.host, self.port,
                                      self.timeout_ms,
                                      self.proto_version,
                                      &self.service))

    def __dealloc__(self):
        self.close()

    def close(self):
        # Idempotent
        cdef CService* sp = self.service.get()
        if sp != NULL:
            check_status(sp.Close())

    def is_connected(self):
        return self.service.get().IsConnected()

    def open_session(self):
        """
        Start a new HiveServer2 session, which may consist of one or more
        operations under a particular user and set of config options

        Parameters
        ----------

        Returns
        -------
        operation : Operation
        """
        cdef Session session = Session(self)

        cdef HS2ClientConfig config
        check_status(self.service.get()
                     .OpenSession(self.user, config, &session.session))
        return session


cdef class Session:
    """
    Interface to a HiveServer2 session (for a particular user and set of
    options). A session may span multiple operations.
    """
    cdef:
        unique_ptr[CSession] session

    cdef readonly:
        Service parent

    def __cinit__(self, Service parent):
        self.parent = parent

    def __dealloc__(self):
        self.close_session()

    cdef close_session(self):
        # Idempotent
        cdef CSession* sp = self.session.get()
        if sp != NULL:
            check_status(sp.Close())

    def close(self):
        """
        Explicitly close the Session, sending any necessary RPCs. Will be
        done automatically on garbage collection
        """
        self.close_session()

    def execute(self, statement):
        """
        Execute a DDL / SQL operation within the context of the active session

        Returns
        -------
        operation : Operation
        """
        cdef Operation operation = Operation(self)
        cdef string c_statement = tobytes(statement)

        check_status(self.session.get()
                     .ExecuteStatement(c_statement, &operation.op))

        return operation

    def execute_sync(self, statement):
        op = self.execute(statement)
        op.wait()
        return op


cdef operation_state_to_string(OperationState state):
    if state == OperationState_INITIALIZED:
        return 'initialized'
    elif state == OperationState_RUNNING:
        return 'running'
    elif state == OperationState_FINISHED:
        return 'finished'
    elif state == OperationState_CANCELED:
        return 'canceled'
    elif state == OperationState_CLOSED:
        return 'closed'
    elif state == OperationState_ERROR:
        return 'error'
    elif state == OperationState_PENDING:
        return 'pending'
    else:
        return 'unknown'


cdef int DEFAULT_BATCHSIZE = 2 ** 10


cdef class Operation:
    """
    Interface to a HiveServer2 operation, typically executing a SQL (DDL or
    DML) statement.
    """
    cdef:
        unique_ptr[COperation] op
        object cached_metadata

    cdef readonly:
        Session parent

    def __cinit__(self, Session parent):
        self.parent = parent
        self.cached_metadata = None

    def __dealloc__(self):
        self.close_operation()

    def cancel(self):
        """
        May be called before the operation has been closed.
        """
        check_status(self.op.get().Cancel())

    def close(self):
        """
        Explicitly close the Operation, sending any necessary RPCs. Will be
        done automatically on garbage collection
        """
        self.close_operation()

    def fetchall_pandas(self, batchsize=None):
        """

        """
        cdef:
            vector[CColumnarRowSet*] c_row_sets
            ColumnarRowSet py_row_set
            Schema schema
            const CColumnType* col_type
            int i

        import pandas as pd

        # The extension class retains ownership of the
        # hs2client::ColumnarRowSet
        batches = self.fetchall_internal(batchsize=batchsize)
        schema = self.schema

        for py_row_set in batches:
            c_row_sets.push_back(py_row_set.data.get())

        # TODO(wesm): consider parallelizing deserialization
        column_names = []
        converted_columns = {}
        for i in range(schema.ncolumns):
            col_name = schema.column(i).name
            col_type = schema.columns[i].type()

            result = ConvertColumnPandas(c_row_sets, i, col_type)

            if col_type.type_id() == ColumnType_TIMESTAMP:
                result = pd.to_datetime(result)
            elif col_type.type_id() == ColumnType_DECIMAL:
                _convert_decimals(result)

            column_names.append(col_name)
            converted_columns[col_name] = result

        return pd.DataFrame(converted_columns, columns=column_names)

    cdef fetchall_internal(self, batchsize=None):
        cdef:
            ColumnarRowSet row_set
            int c_batchsize
            c_bool has_more_rows

        if batchsize is not None:
            c_batchsize = batchsize
        else:
            # TODO(wesm): consider picking a default as a function of the
            # number of columns in the result set (e.g. 2 ** 16 /
            # log2(n_columns)). Investigate performance implications
            c_batchsize = DEFAULT_BATCHSIZE

        cdef list batches = []

        has_more_rows = True
        while has_more_rows:
            row_set = ColumnarRowSet()
            check_status(self.op.get()
                         .Fetch(c_batchsize, FetchOrientation_NEXT,
                                &row_set.data, &has_more_rows))

            batches.append(row_set)

        return batches

    cdef close_operation(self):
        # Idempotent
        cdef COperation* optr = self.op.get()
        if optr != NULL:
            check_status(optr.Close())

    def get_state(self):
        cdef OperationState state
        check_status(self.op.get().GetState(&state))
        return operation_state_to_string(state)

    def get_log(self):
        cdef string log
        check_status(self.op.get().GetLog(&log))
        return frombytes(log)

    def get_profile(self):
        cdef string profile
        check_status(self.op.get().GetProfile(&profile))
        return frombytes(profile)

    def wait(self, timeout_seconds=0):
        if self.is_finished:
            return

        interval = 0.05
        time.sleep(interval)
        total_sleep = interval
        while not self.is_finished:
            interval *= 2
            time.sleep(interval)
            total_sleep += interval
            if timeout_seconds > 0 and total_sleep > timeout_seconds:
                raise Exception('Operation timed out')

    property schema:

        def __get__(self):
            cdef Schema metadata

            if self.cached_metadata is not None:
                return self.cached_metadata

            metadata = Schema()
            check_status(self.op.get().GetResultSetMetadata(&metadata.columns))
            self.cached_metadata = metadata
            return metadata

    property is_finished:
        """
        Returns True if the operation's state can be determined and is found to
        be in the FINISHED state. The result set may be empty
        """

        def __get__(self):
            return self.op.get().HasResultSet()


cdef class ColumnarRowSet:
    cdef:
        unique_ptr[CColumnarRowSet] data


cdef class ColumnType:
    cdef:
        const CColumnType* base
        object parent

    def __cinit__(self, parent):
        self.parent = parent

    property short_repr:

        def __get__(self):
            return self.name

    property name:

        def __get__(self):
            return frombytes(self.base.ToString())


cdef class PrimitiveType(ColumnType):
    pass


cdef class CharacterType(PrimitiveType):
    cdef:
        const CCharacterType* subtype

    property short_repr:

        def __get__(self):
            return '{0}({1})'.format(self.name, self.max_length)

    property max_length:

        def __get__(self):
            return self.subtype.max_length()


cdef class DecimalType(PrimitiveType):
    cdef:
        const CDecimalType* subtype

    property short_repr:

        def __get__(self):
            return '{0}({1}, {2})'.format(self.name, self.precision,
                                          self.scale)

    property precision:

        def __get__(self):
            return self.subtype.precision()

    property scale:

        def __get__(self):
            return self.subtype.scale()


cdef class ColumnDesc:
    """
    The descriptor for a single column in a result set in HiveServer2
    """
    cdef:
        const CColumnDesc* desc
        Schema parent

    def __cinit__(self, Schema parent):
        self.parent = parent

    def __repr__(self):
        return ('HS2Column(name={0}, type={1!r})'.
                format(self.name, self.type.short_repr))

    property name:

        def __get__(self):
            return frombytes(self.desc.column_name())

    property position:

        def __get__(self):
            return self.desc.position()

    property comment:

        def __get__(self):
            if self.desc.comment().size() == 0:
                return None
            else:
                return frombytes(self.desc.comment())

    property type:

        def __get__(self):
            cdef:
                PrimitiveType base_type
                CharacterType char_type
                DecimalType dec_type

                const CColumnType* type_ptr = self.desc.type()

            if type_ptr.type_id() == ColumnType_DECIMAL:
                dec_type = DecimalType(self)
                dec_type.base = type_ptr
                dec_type.subtype = self.desc.GetDecimalType()
                return dec_type
            elif (type_ptr.type_id() == ColumnType_CHAR or
                  type_ptr.type_id() == ColumnType_VARCHAR):
                char_type = CharacterType(self)
                char_type.base = type_ptr
                char_type.subtype = self.desc.GetCharacterType()
                return char_type
            else:
                base_type = PrimitiveType(self)
                base_type.base = type_ptr
                return base_type


cdef class Schema:
    """
    Interface for HiveServer2 result set metadata
    """
    cdef:
        vector[CColumnDesc] columns

    def __len__(self):
        return self.columns.size()

    def __getitem__(self, i):
        return self.column(i)

    property ncolumns:
        def __get__(self):
            return len(self)

    def column(self, int i):
        cdef ColumnDesc desc = ColumnDesc(self)

        if i < 0 or i >= (<int> self.columns.size()):
            raise IndexError('Column index {0} is out of bounds'.format(i))

        desc.desc = &self.columns[i]
        return desc


cdef _convert_decimals(cnp.ndarray[object] arr):
    cdef:
        int i, n = len(arr)

    for i in range(n):
        if arr[i] is None:
            continue
        arr[i] = Decimal(arr[i])
