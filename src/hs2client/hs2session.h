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

#ifndef HS2CLIENT_HS2SESSION_H
#define HS2CLIENT_HS2SESSION_H

#include "hs2client/hs2service.h"
#include "hs2client/operation.h"

namespace hs2client {

struct HS2SessionInfo;

/**
 * Manages a single HiveServer2 session.
 */
class HS2Session {
 public:
  HS2Session(HS2Service service,
      HS2SessionInfo session_info,
      HS2ClientConfig config,
      int retries = 3);

  // Disable copy and assignment.
  HS2Session(HS2Session const&) = delete;
  HS2Session& operator=(HS2Session const&) = delete;

  Status Close();

  std::shared_ptr<Operation> ExecuteStatement(const std::string& statement,
      HS2ClientConfig config = std::map<std::string, std::string>());

  std::shared_ptr<Operation> GetDatabases(const std::string& schema = ".*");

  std::shared_ptr<Operation> GetTables(const std::string& database = ".*",
      const std::string& table_like = ".*");

  std::shared_ptr<Operation> GetTableSchema(const std::string& table,
      const std::string& database = ".*");

  std::shared_ptr<Operation> GetFunctions(const std::string& database = ".*");

  std::shared_ptr<Operation> DatabaseExists(const std::string& database = ".*");

  std::shared_ptr<Operation> TableExists(const std::string& table,
      const std::string& database = ".*");

  bool Ping();
};

}

#endif // HS2CLIENT_HS2SESSION_H
