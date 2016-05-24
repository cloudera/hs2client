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

#ifndef HS2CLIENT_UTIL_H
#define HS2CLIENT_UTIL_H

#include <string>

#include "hs2client/operation.h"

namespace hs2client {

// Utility functions. Intended primary for testing purposes - clients should not
// rely on stability of the behavior or API of these functions.
class Util {
 public:
  // Fetches the operation's results and returns them in a nicely formatted string.
  static void PrintResults(const Operation* op, std::ostream& out);
};

} // namespace hs2client

#endif // HS2CLIENT_UTIL_H
