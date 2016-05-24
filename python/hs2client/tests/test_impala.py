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

import getpass
import os

from pandas.util.testing import assert_frame_equal
import numpy as np
import pandas as pd

import pytest

from hs2client.compat import Decimal
from hs2client.tests.common import ExampleEnv, random_table_name, insert_tuples
import hs2client as hs2


TEST_HOST = os.environ.get('HS2_TEST_IMPALA_HOST', 'localhost')
TEST_PORT = int(os.environ.get('HS2_TEST_IMPALA_PORT', 21050))
TEST_USER = os.environ.get('HS2_TEST_IMPALA_USER', getpass.getuser())


def test_service_session_scope():
    svc = hs2.connect(TEST_HOST, TEST_PORT, TEST_USER)
    session = svc.open_session()
    svc = None

    op = session.execute('show tables')  # noqa
    session = None


@pytest.fixture(scope='module')
def env1(request):
    env = ExampleEnv(TEST_HOST, TEST_PORT, TEST_USER)
    env.setup()
    request.addfinalizer(env.teardown)
    return env


def test_result_metadata(env1):
    # As of Impala 2.5, only supports scalar types in result set
    # metadata. hive can return more types; will need to test this
    # separately

    table_name = random_table_name()

    sql = """
create table {0} (
f0 tinyint comment 'f0',
f1 smallint,
f2 int,
f3 bigint,
f4 float,
f5 double,
f6 char(10),
f7 varchar(20),
f8 string,
f9 decimal(12, 2),
f10 timestamp,
f11 boolean
)
""".format(table_name)

    env1.session.execute_sync(sql)

    op = env1.select_all(table_name)

    schema = op.schema
    expected = [
        # name, type, comment
        ('f0', 'TINYINT', 'f0'),
        ('f1', 'SMALLINT', None),
        ('f2', 'INT', None),
        ('f3', 'BIGINT', None),
        ('f4', 'FLOAT', None),
        ('f5', 'DOUBLE', None),
        ('f6', 'CHAR', None),
        ('f7', 'VARCHAR', None),
        ('f8', 'STRING', None),
        ('f9', 'DECIMAL', None),
        ('f10', 'TIMESTAMP', None),
        ('f11', 'BOOLEAN', None)
    ]

    assert len(schema) == len(expected)
    assert schema.ncolumns == len(schema)

    for i, attrs in enumerate(expected):
        name, typename, comment = attrs

        desc = schema[i]

        coltype = desc.type

        assert desc.name == name
        assert coltype.name == typename

        # Comments seem to not be coming back
        # assert desc.comment == comment

    # test decimal stuff
    decimal = schema[9].type
    assert decimal.precision == 12
    assert decimal.scale == 2

    # test char/varchar stuff
    char = schema[6].type
    assert char.max_length == 10

    varchar = schema[7].type
    assert varchar.max_length == 20


# ---------------------------------------------------------------------
# Well-behaved pandas types: float, double, string, timestamp


def test_pandas_fetch_numeric(env1):
    all_names = []
    all_types = []
    all_data = []
    ex_dtypes = []

    def push(names, types, ex_dtypes, data):
        all_names.extend(names)
        all_types.extend(types)
        all_data.extend(data)
        ex_dtypes.extend(dtypes)

    K = 20
    length = 100

    # Integer without nulls
    names = ['i0', 'i1', 'i2', 'i3']
    types = ['tinyint', 'smallint', 'int', 'bigint']
    dtypes = ['i1', 'i2', 'i4', 'i8']
    data = [[1, 2, 3, 4, 5] * K] * 4

    push(names, types, dtypes, data)

    expected = pd.DataFrame(index=range(length))

    for name, np_type, data in zip(names, dtypes, data):
        expected[name] = np.array(data, dtype=np_type)

    # Integers with nulls
    names = ['i4', 'i5', 'i6', 'i7']
    types = ['tinyint', 'smallint', 'int', 'bigint']
    dtypes = ['f8'] * 4
    data = [[1, 2, None, 4, 5] * K] * 4

    for name, np_type, arr in zip(names, dtypes, data):
        # leave this conversion to pandas (automatically -> float64)
        expected[name] = arr

    push(names, types, dtypes, data)

    # Floating point cases
    names = ['float0', 'float1', 'double0', 'double1']
    types = ['float', 'float', 'double', 'double']
    dtypes = ['f4', 'f4', 'f8', 'f8']
    data = [
        [-0.5, 0, None, 0.5, 1] * K,
        [-0.5, 0, 0.5, 1, 1.5] * K,
    ] * 2

    for name, ctype, dtype, arr in zip(names, types, dtypes, data):
        expected[name] = np.array(arr, dtype=dtype)

    push(names, types, dtypes, data)

    # We can do this in one shot
    result = _roundtrip_data(env1, all_names, all_types, all_data)

    assert_frame_equal(result, expected)

    for name, ex_dtype in zip(all_names, ex_dtypes):
        assert result[name].dtype == ex_dtype


def test_pandas_fetch_boolean(env1):
    # Test with nulls and without
    K = 20
    data = [
        [True, False, True, False, True] * K,
        [True, None, None, False, True] * K
    ]

    colnames = ['f0', 'f1']
    coltypes = ['boolean', 'boolean']

    # We can do this in one shot
    expected = pd.DataFrame({name: np.array(arr)
                             for name, arr in zip(colnames, data)},
                            columns=colnames)

    result = _roundtrip_data(env1, colnames, coltypes, data)

    assert_frame_equal(result, expected)

    # Boolean + nulls in pandas becomes numpy object
    assert result['f0'].dtype == np.bool_
    assert result['f1'].dtype == np.object_


def test_pandas_fetch_string(env1):
    K = 20
    data = [
        ['long string', 'foo', None, 'foo', 'long string'] * K
    ]
    colnames = ['f0']
    coltypes = ['string', 'boolean']

    # We can do this in one shot
    expected = pd.DataFrame({colnames[0]: data[0]})
    result = _roundtrip_data(env1, colnames, coltypes, data)

    assert_frame_equal(result, expected)

    # Test string interning
    c = result['f0']
    assert c[0] is c[4]
    assert c[1] is c[3]


def test_pandas_fetch_timestamp(env1):
    K = 20
    v = '2000-01-01 12:34:56.123456'
    data = [
        [v, None, '2001-01-01', v] * K
    ]
    colnames = ['f0']
    coltypes = ['timestamp']

    # We can do this in one shot
    expected = pd.DataFrame({colnames[0]: pd.to_datetime(data[0])})
    result = _roundtrip_data(env1, colnames, coltypes, data)

    assert_frame_equal(result, expected)


def test_pandas_fetch_decimal(env1):
    K = 20
    values = [-1.5, None, 0, 1.5] * K
    colnames = ['f0']
    coltypes = ['decimal(12,2)']

    # We can do this in one shot
    expected = pd.DataFrame({
        colnames[0]: np.array([Decimal(x) if x is not None else x
                               for x in values], dtype=object)
    })
    result = _roundtrip_data(env1, colnames, coltypes, [values])

    assert_frame_equal(result, expected)


def _roundtrip_data(env, colnames, coltypes, column_data, batchsize=16):
    tname = random_table_name()
    env.create_table(tname, zip(colnames, coltypes))
    insert_tuples(env.session, tname, zip(*column_data))

    op = env.select_all(tname)
    return op.fetchall_pandas(batchsize=16)
