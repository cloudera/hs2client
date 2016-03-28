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

#include "hs2client/columnar-row-set.h"

#include "hs2client/thrift-internal.h"

#include "gen-cpp/TCLIService.h"

namespace hs2 = apache::hive::service::cli::thrift;
using std::unique_ptr;

namespace hs2client {

ColumnarRowSet::ColumnarRowSet(ColumnarRowSetImpl* impl) : impl_(impl) {}

ColumnarRowSet::~ColumnarRowSet() = default;

unique_ptr<BoolColumn> ColumnarRowSet::GetBoolCol(int i) const {
  hs2::TBoolColumn& bool_col = impl_->resp.results.columns[i].boolVal;
  unique_ptr<BoolColumn> col(new BoolColumn(
      reinterpret_cast<const uint8_t*>(bool_col.nulls.c_str()),
      &bool_col.values));
  return col;
}

unique_ptr<ByteColumn> ColumnarRowSet::GetByteCol(int i) const {
  hs2::TByteColumn& byte_col = impl_->resp.results.columns[i].byteVal;
  unique_ptr<ByteColumn> col(new ByteColumn(
      reinterpret_cast<const uint8_t*>(byte_col.nulls.c_str()),
      &byte_col.values));
  return col;
}

unique_ptr<Int16Column> ColumnarRowSet::GetInt16Col(int i) const {
  hs2::TI16Column& int16_col = impl_->resp.results.columns[i].i16Val;
  unique_ptr<Int16Column> col(new Int16Column(
      reinterpret_cast<const uint8_t*>(int16_col.nulls.c_str()),
      &int16_col.values));
  return col;
}

unique_ptr<Int32Column> ColumnarRowSet::GetInt32Col(int i) const {
  hs2::TI32Column& int32_col = impl_->resp.results.columns[i].i32Val;
  unique_ptr<Int32Column> col(new Int32Column(
      reinterpret_cast<const uint8_t*>(int32_col.nulls.c_str()),
      &int32_col.values));
  return col;
}

unique_ptr<Int64Column> ColumnarRowSet::GetInt64Col(int i) const {
  hs2::TI64Column& int64_col = impl_->resp.results.columns[i].i64Val;
  unique_ptr<Int64Column> col(new Int64Column(
      reinterpret_cast<const uint8_t*>(int64_col.nulls.c_str()),
      &int64_col.values));
  return col;
}

unique_ptr<StringColumn> ColumnarRowSet::GetStringCol(int i) const {
  hs2::TStringColumn& string_col = impl_->resp.results.columns[i].stringVal;
  unique_ptr<StringColumn> col(new StringColumn(
      reinterpret_cast<const uint8_t*>(string_col.nulls.c_str()),
      &string_col.values));
  return col;
}

unique_ptr<BinaryColumn> ColumnarRowSet::GetBinaryCol(int i) const {
  hs2::TBinaryColumn& binary_col = impl_->resp.results.columns[i].binaryVal;
  unique_ptr<BinaryColumn> col(new BinaryColumn(
      reinterpret_cast<const uint8_t*>(binary_col.nulls.c_str()),
      &binary_col.values));
  return col;
}

} // namespace hs2client
