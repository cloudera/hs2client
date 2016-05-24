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

from libc.stdint cimport *
from libcpp cimport bool as c_bool
from libcpp.string cimport string
from libcpp.vector cimport vector

# This must be included for cerr and other things to work
cdef extern from "<iostream>":
    pass

cdef extern from "<memory>" namespace "std" nogil:

    cdef cppclass unique_ptr[T]:
        unique_ptr()
        unique_ptr(T*)
        T* get()
        void reset()
        void reset(T* p)
        T* release()

    cdef cppclass shared_ptr[T]:
        shared_ptr()
        shared_ptr(T*)
        T* get()
        void reset()
        void reset(T* p)

cdef extern from "hs2client/api.h" namespace "hs2client" nogil:

    #----------------------------------------------------------------------
    # Status / errors

    cdef Status Status_OK "Status::OK"()

    cdef cppclass Status:
        const string& GetMessage()

        c_bool ok()
        c_bool IsStillExecuting()
        c_bool IsError()
        c_bool IsInvalidHandle()

    #----------------------------------------------------------------------
    # Column types

    cdef cppclass Column:
        const uint8_t* nulls()
        int64_t length()
        c_bool IsNull(int)

    cdef cppclass BoolColumn(Column):

        const vector[c_bool]& data()

    cdef cppclass ByteColumn(Column):

        const vector[int8_t]& data()

    cdef cppclass Int16Column(Column):

        const vector[int16_t]& data()

    cdef cppclass Int32Column(Column):

        const vector[int32_t]& data()

    cdef cppclass Int64Column(Column):

        const vector[int64_t]& data()

    cdef cppclass StringColumn(Column):

        const vector[string]& data()

    cdef cppclass BinaryColumn(Column):

        const vector[string]& data()

    cdef cppclass CColumnarRowSet" hs2client::ColumnarRowSet":
        unique_ptr[T] GetCol[T](int i)

        unique_ptr[BoolColumn] GetBoolCol(int i)
        unique_ptr[ByteColumn] GetByteCol(int i)
        unique_ptr[Int16Column] GetInt16Col(int i)
        unique_ptr[Int32Column] GetInt32Col(int i)
        unique_ptr[Int64Column] GetInt64Col(int i)
        unique_ptr[StringColumn] GetStringCol(int i)
        unique_ptr[BinaryColumn] GetBinaryCol(int i)

    #----------------------------------------------------------------------
    # Types and metadata

    enum ColumnTypeId" hs2client::ColumnType::TypeId":
        ColumnType_BOOLEAN " hs2client::ColumnType::TypeId::BOOLEAN"
        ColumnType_TINYINT " hs2client::ColumnType::TypeId::TINYINT"
        ColumnType_SMALLINT " hs2client::ColumnType::TypeId::SMALLINT"
        ColumnType_INT " hs2client::ColumnType::TypeId::INT"
        ColumnType_BIGINT " hs2client::ColumnType::TypeId::BIGINT"
        ColumnType_FLOAT " hs2client::ColumnType::TypeId::FLOAT"
        ColumnType_DOUBLE " hs2client::ColumnType::TypeId::DOUBLE"
        ColumnType_STRING " hs2client::ColumnType::TypeId::STRING"
        ColumnType_TIMESTAMP " hs2client::ColumnType::TypeId::TIMESTAMP"
        ColumnType_BINARY " hs2client::ColumnType::TypeId::BINARY"
        ColumnType_ARRAY " hs2client::ColumnType::TypeId::ARRAY"
        ColumnType_MAP " hs2client::ColumnType::TypeId::MAP"
        ColumnType_STRUCT " hs2client::ColumnType::TypeId::STRUCT"
        ColumnType_UNION " hs2client::ColumnType::TypeId::UNION"
        ColumnType_USER_DEFINED " hs2client::ColumnType::TypeId::USER_DEFINED"
        ColumnType_DECIMAL " hs2client::ColumnType::TypeId::DECIMAL"
        ColumnType_NULL_TYPE " hs2client::ColumnType::TypeId::NULL_TYPE"
        ColumnType_DATE " hs2client::ColumnType::TypeId::DATE"
        ColumnType_VARCHAR " hs2client::ColumnType::TypeId::VARCHAR"
        ColumnType_CHAR " hs2client::ColumnType::TypeId::CHAR"
        ColumnType_INVALID " hs2client::ColumnType::TypeId::INVALID"

    cdef cppclass CColumnType" hs2client::ColumnType":
        const ColumnTypeId type_id()
        const string ToString()

    cdef cppclass CPrimitiveType" hs2client::PrimitiveType"(CColumnType):
        CPrimitiveType(const ColumnTypeId& type_id)

    cdef cppclass CCharacterType" hs2client::CharacterType"(CPrimitiveType):
        CCharacterType(const ColumnTypeId& type_id, int max_length)
        const int max_length()

    cdef cppclass CDecimalType" hs2client::DecimalType"(CPrimitiveType):
        CDecimalType(const ColumnTypeId& type_id, int precision, int scale)
        const int precision()
        const int scale()

    cdef cppclass CColumnDesc" hs2client::ColumnDesc":
        CColumnDesc(const string& column_name, unique_ptr[CColumnType] type,
                    int position, const string& comment)

        const string& column_name()
        const CColumnType* type()
        const int position()
        const string& comment()

        const CPrimitiveType* GetPrimitiveType()
        const CCharacterType* GetCharacterType()
        const CDecimalType* GetDecimalType()

    #----------------------------------------------------------------------
    # Service

    cdef cppclass HS2ClientConfig:
        HS2ClientConfig()

        void SetOption(const string& key, const string& value)
        c_bool GetOption(const string& key, string* value_out)

    enum ProtocolVersion" hs2client::ProtocolVersion":
        Protocol_V1 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V1"
        Protocol_V2 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V2"
        Protocol_V3 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V3"
        Protocol_V4 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V4"
        Protocol_V5 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V5"
        Protocol_V6 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V6"
        Protocol_V7 " hs2client::ProtocolVersion::HS2CLIENT_PROTOCOL_V7"

    cdef cppclass CService" hs2client::Service":

        @staticmethod
        Status Connect(const string& host, int port, int conn_timeout,
                       ProtocolVersion protocol,
                       unique_ptr[CService]* service)

        Status Close()

        c_bool IsConnected()

        void SetRecvTimeout(int timeout)
        void SetSendTimeout(int timeout)

        Status OpenSession(const string& user, const HS2ClientConfig& config,
                           unique_ptr[CSession]* session)


    cdef cppclass CSession" hs2client::Session":

        Status Close()
        Status ExecuteStatement(const string& statement,
                                unique_ptr[COperation]* operation)

        Status ExecuteStatement(const string& statement,
                                const HS2ClientConfig& conf_overlay,
                                unique_ptr[COperation]* operation)


    enum OperationState" hs2client::Operation::State":
        OperationState_INITIALIZED " hs2client::Operation::State::INITIALIZED"
        OperationState_RUNNING " hs2client::Operation::State::RUNNING"
        OperationState_FINISHED " hs2client::Operation::State::FINISHED"
        OperationState_CANCELED " hs2client::Operation::State::CANCELED"
        OperationState_CLOSED " hs2client::Operation::State::CLOSED"
        OperationState_ERROR " hs2client::Operation::State::ERROR"
        OperationState_UNKNOWN " hs2client::Operation::State::UNKNOWN"
        OperationState_PENDING " hs2client::Operation::State::PENDING"

    enum FetchOrientation" hs2client::FetchOrientation":
        FetchOrientation_NEXT " hs2client::FetchOrientation::NEXT"

    cdef cppclass COperation" hs2client::Operation":

        Status GetState(OperationState* out)
        Status GetLog(string* out)
        Status GetProfile(string* out)
        Status GetResultSetMetadata(vector[CColumnDesc]* out)

        # Fetches 1024 rows by default currently
        Status Fetch(unique_ptr[CColumnarRowSet]* results,
                     c_bool* has_more_rows)

        Status Fetch(int max_rows, FetchOrientation orientation,
                     unique_ptr[CColumnarRowSet]* results,
                     c_bool* has_more_rows)

        # May be called after successfully creating the operation and before
        # calling Close.
        Status Cancel()

        Status Close()

        c_bool HasResultSet()

        c_bool IsColumnar()
