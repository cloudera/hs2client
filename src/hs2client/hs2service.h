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
  // The client calling Connect has ownership of the new HS2Service that is
  // created.
  // Executing RPCs with an HS2Session or Operation corresponding to a particular
  // HS2Service after that HS2Service has been closed or deleted in undefined.
  static Status Connect(const std::string& host, int port, int timeout, bool use_ssl,
      std::unique_ptr<HS2Service>* out);

  ~HS2Service();

  // Disable copy and assignment.
  HS2Service(HS2Service const&) = delete;
  HS2Service& operator=(HS2Service const&) = delete;

  Status Close();

  Status Reconnect();

  // The client calling OpenSession has ownership of the HS2Session that is created.
  // Executing RPCs with an Operation corresponding to a particular HS2Session after
  // that HS2Session has been closed or deleted is undefined.
  Status OpenSession(const std::string& user, const HS2ClientConfig& config,
      std::unique_ptr<HS2Session>* out);

 private:
  struct Impl;

  HS2Service(HS2Service::Impl* impl);

  std::unique_ptr<HS2Service::Impl> impl_;
};

}

#endif // HS2CLIENT_HS2SERVICE_H
