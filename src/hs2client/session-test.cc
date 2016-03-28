// Copyright 2016 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "hs2client/session.h"

#include <gtest/gtest.h>
#include <memory>

#include "hs2client/test-util.h"

using namespace hs2client;
using namespace std;

class SessionTest : public HS2ClientTest {};

TEST_F(SessionTest, TestSessionConfig) {
  // Create a table in TEST_DB.
  const string& TEST_TBL = "hs2client_test_table";
  unique_ptr<Operation> create_table_op;
  EXPECT_OK(session_->ExecuteStatement("create table " + TEST_TBL +
      " (int_col int, string_col string)", &create_table_op));
  EXPECT_OK(create_table_op->Close());

  // Start a new session with the use:database session option.
  string user = "user";
  HS2ClientConfig config_use;
  config_use.SetOption("use:database", TEST_DB);
  unique_ptr<Session> session_ok;
  EXPECT_OK(service_->OpenSession(user, config_use, &session_ok));

  // Ensure the use:database worked and we can access the table.
  unique_ptr<Operation> select_op;
  EXPECT_OK(session_ok->ExecuteStatement("select * from " + TEST_TBL, &select_op));
  EXPECT_OK(select_op->Close());
  EXPECT_OK(session_ok->Close());

  // Start another session without use:database.
  HS2ClientConfig config_no_use;
  unique_ptr<Session> session_error;
  EXPECT_OK(service_->OpenSession(user, config_no_use, &session_error));

  // Ensure the we can't access the table.
  unique_ptr<Operation> select_op_error;
  EXPECT_ERROR(session_error->ExecuteStatement("select * from " + TEST_TBL,
      &select_op_error));
  EXPECT_OK(session_error->Close());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
