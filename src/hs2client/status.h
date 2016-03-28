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

#ifndef HS2CLIENT_STATUS_H
#define HS2CLIENT_STATUS_H

#include <string>

namespace hs2client {

// Based on HiveServer2 TStatusCode.
enum class StatusCode {
  // No errors occurred. May have a message set if we got a
  // SUCCESS_WITH_INFO_STATUS from hs2.
  Success,

  // Action could not be completed because the operation is still executing.
  StillExecuting,

  // General errors, usually has a message set with more details.
  Error,

  // The session or operation handle is invalid, eg. because it has been closed.
  InvalidHandle,
};

class Status {
 public:
  static Status OK(const std::string& msg = "");
  static Status StillExecuting();
  static Status Error(const std::string& msg);
  static Status InvalidHandle();

  bool ok() const;
  bool IsStillExecuting() const;
  bool IsError() const;
  bool IsInvalidHandle() const;

  const std::string& GetMessage() const { return msg_; }

 private:
  Status(StatusCode code, const std::string& msg = "");

  StatusCode code_;

  std::string msg_;
};

} // namespace hs2client

#endif // HS2CLIENT_STATUS_H
