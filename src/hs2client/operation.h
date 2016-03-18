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

#ifndef HS2CLIENT_OPERATION_H
#define HS2CLIENT_OPERATION_H

#include "hs2client/columnar_row_set.h"
#include "hs2client/status.h"

namespace hs2client {

class HS2Session;

enum FetchOrientation {
  FETCH_NEXT,
  FETCH_PRIOR,
  FETCH_RELATIVE,
  FETCH_ABSOLUTE,
  FETCH_FIRST,
  FETCH_LAST
};

/**
 * Represents a single HiveServer2 operation. Used to monitor the status of an operation
 * and to retrieve its results.
 */
class Operation {
 public:
  ~Operation();

  // Disable copy and assignment.
  Operation(Operation const&) = delete;
  Operation& operator=(Operation const&) = delete;

  Status Execute(bool async = false);

  bool HasResultSet();

  Status GetStatus();

  Status GetLog(std::string* out);

  Status Cancel();

  Status Close();

  Status GetProfile(std::string* out);

  Status Fetch(std::unique_ptr<ColumnarRowSet>* out, int max_rows = 1024,
      FetchOrientation orientation = FetchOrientation::FETCH_NEXT);

  bool IsColumnar();

 private:
  struct Impl;

  friend class HS2Session;

  Operation(HS2Session* session);

  HS2Session* session;

  std::unique_ptr<Operation::Impl> impl_;
};

}

#endif // HS2CLIENT_OPERATION_H
