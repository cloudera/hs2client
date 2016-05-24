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

from hs2client.compat import guid
import hs2client as hs2

TEST_DB = 'hs2client_test_db'


class ExampleEnv(object):

    def __init__(self, host, port, user, test_db=TEST_DB, protocol='v7'):
        self.service = hs2.Service(host, port, user, protocol=protocol)
        self.session = self.service.open_session()

        self.test_db = test_db

    def setup(self):
        _drop_database(self.session, self.test_db)
        _create_database(self.session, self.test_db)
        _use_database(self.session, self.test_db)

    def teardown(self):
        _use_database(self.session, 'default')
        _drop_database(self.session, self.test_db)

    def create_table(self, table_name, schema):
        tokens = []
        for name, ctype in schema:
            tokens.append('{0} {1}'.format(name, ctype))

        formatted_schema = ', '.join(tokens)

        stmt = 'create table {0} ({1})'.format(table_name, formatted_schema)
        self.session.execute(stmt)

    def select_all(self, table_name):
        return self.session.execute('select * from {0}'.format(table_name))


def _drop_database(session, name):
    stmt = 'drop database if exists {0} cascade'.format(name)
    session.execute(stmt)


def _create_database(session, name):
    stmt = 'create database {0}'.format(name)
    session.execute(stmt)


def _use_database(session, name):
    stmt = 'use {0}'.format(name)
    session.execute(stmt)


def random_table_name():
    return 'hs2test_{0}'.format(guid())


def insert_tuples(session, table, values):
    tokens = []
    for tup in values:
        row = '({0})'.format(', '.join('NULL' if x is None else repr(x)
                                       for x in tup))
        tokens.append(row)

    formatted_values = ','.join(tokens)
    stmt = 'INSERT INTO {0} VALUES {1}'.format(table, formatted_values)
    session.execute_sync(stmt)
