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

#ifndef HS2CLIENT_HS2SERVICE_H
#define HS2CLIENT_HS2SERVICE_H

#include <map>
#include <memory>
#include <string>

#include "hs2client/status.h"

namespace hs2client {

class HS2Session;

typedef std::map<std::string, std::string> HS2ClientConfig;

/**
 * Manages a connection to a HiveServer2 server.
 */
class HS2Service {
 public:
  static Status Connect(const std::string& host, int port, int timeout, bool use_ssl,
      std::unique_ptr<HS2Service>* out);

  // Disable copy and assignment.
  HS2Service(HS2Service const&) = delete;
  HS2Service& operator=(HS2Service const&) = delete;

  Status Close();

  Status Reconnect();

  Status OpenSession(const std::string& user, HS2ClientConfig config,
      std::unique_ptr<HS2Session>* out);

 private:
  HS2Service(int retries = 3);
};

}

#endif // HS2CLIENT_HS2SERVICE_H
