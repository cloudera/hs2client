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

enum class StatusCode {
  SuccessStatus,
  SuccessWithInfoStatus,
  StillExecutingStatus,
  ErrorStatus,
  InvalidHandleStatus,
};

class Status {
 public:
  // Create a success status.
  Status();

  // Copy the specified status.
  Status(const Status& s);
  void operator=(const Status& s);

  static Status Success();
  static Status SuccessWithInfo(const std::string& msg);
  static Status StillExecuting();
  static Status Error(const std::string& msg);
  static Status InvalidHandle();

  bool IsSuccess();
  bool IsSuccessWithInfo();
  bool IsStillExecuting();
  bool IsError();
  bool IsInvalidHandle();

  bool ok();

  // Return a string representation of this status suitable for printing.
  std::string ToString();
};

}

#endif // HS2CLIENT_STATUS_H
