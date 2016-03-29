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

  bool HasResultSet();

  Status GetStatus();

  Status GetLog(std::string* out);

  Status Cancel();

  Status Close();

  Status GetProfile(std::string* out);

  Status Fetch(std::unique_ptr<ColumnarRowSet>* out);

  Status Fetch(int max_rows, FetchOrientation orientation,
      std::unique_ptr<ColumnarRowSet>* out);

  bool IsColumnar();

 private:
  struct OperationImpl;

  friend class HS2Session;

  Operation(HS2Session* session);

  // This ptr is owned by the client that created the HS2Session. It is up to the client
  // to ensure that the session is not deleted while there are still active operations.
  HS2Session* session_;

  std::unique_ptr<OperationImpl> impl_;
};

}

#endif // HS2CLIENT_OPERATION_H
