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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hs2client {

struct ColumnarRowSetInfo;

struct Column {
  int64_t length;
  const void* nulls;
};

struct BoolColumn : public Column {
  const std::vector<bool>* data;
};

struct ByteColumn : public Column {
  const std::vector<int8_t>* data;
};

struct Int16Column : public Column {
  const std::vector<int16_t>* data;
};

struct Int32Column : public Column {
  const std::vector<int32_t>* data;
};

struct Int64Column : public Column {
  const std::vector<int64_t>* data;
};

struct DoubleColumn : public Column {
  const std::vector<double>* data;
};

struct StringColumn : public Column {
  const std::vector<std::string>* data;
};

struct BinaryColumn : public Column {
  const std::vector<std::string>* data;
};

class ColumnarRowSet {
 public:
  BoolColumn GetBool(int i);
  ByteColumn GetByte(int i);
  Int16Column GetInt16(int i);
  Int32Column GetInt32(int i);
  Int64Column GetInt64(int i);
  StringColumn GetString(int i);
  BinaryColumn GetBinary(int i);

 private:
  std::unique_ptr<ColumnarRowSetInfo> info_;
};

}
