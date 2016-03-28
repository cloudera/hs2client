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

#include "hs2client/status.h"

using std::string;

namespace hs2client {

Status Status::OK(const string& msg) {
  return Status(StatusCode::Success, msg);
}

Status Status::StillExecuting() {
  return Status(StatusCode::StillExecuting);
}

Status Status::Error(const string& msg) {
  return Status(StatusCode::Error, msg);
}

Status Status::InvalidHandle() {
  return Status(StatusCode::InvalidHandle);
}

Status::Status(StatusCode code, const string& msg) : code_(code), msg_(msg) {}

bool Status::ok() const {
  return code_ == StatusCode::Success;
}

bool Status::IsStillExecuting() const {
  return code_ == StatusCode::StillExecuting;
}

bool Status::IsError() const {
  return code_ == StatusCode::Error;
}

bool Status::IsInvalidHandle() const {
  return code_ == StatusCode::InvalidHandle;
}

} // namespace hs2client
