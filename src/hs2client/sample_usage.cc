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

#include "hs2client/hs2service.h"
#include "hs2client/hs2session.h"
#include "hs2client/status.h"

using namespace hs2client;

int main(int argc, char** argv) {

  // Connect to the server.
  std::string host = "host";
  int port = 0;
  int timeout = 10;
  bool use_ssl = false;
  std::unique_ptr<HS2Service> service;
  Status status = HS2Service::Connect(host, port, timeout, use_ssl, &service);

  // Open a session.
  std::string user = "user";
  HS2ClientConfig config = std::map<std::string, std::string>();
  std::unique_ptr<HS2Session> session;
  status = service->OpenSession(user, config, &session);

  // Execute a statement.
  std::string statement = "SELECT * FROM test";
  std::shared_ptr<Operation> op = session->ExecuteStatement(statement);
  bool async = true;
  op->Execute(async);

  return 0;
}
