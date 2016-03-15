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

#ifndef HS2CLIENT_BATCH_H
#define HS2CLIENT_BATCH_H

#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace hs2client {

struct BatchInfo;

class ColumnValue {
 public:
  bool GetBool();
  unsigned char GetByte();
  int16_t GetSmallInt();
  int32_t GetInt();
  int64_t GetBigInt();
  int64_t GetTimestamp();
  float GetFloat();
  double GetDouble();
  std::string& GetString();
};

typedef std::list<std::list<ColumnValue>>::iterator BatchIterator;


/**
 * Represents one unit of results to be returned by a HiveServer2 operation.
 */
class Batch {
 public:
  virtual std::list<ColumnValue> pop() = 0;

  virtual BatchIterator iterator() = 0;

  virtual int NumBatchesRemaining() = 0;
};

/**
 * Column oriented batch of results.
 */
class CBatch : public Batch {
 public:
  CBatch(BatchInfo batch_info);

  virtual std::list<ColumnValue> pop();

  virtual BatchIterator iterator();

  virtual int NumBatchesRemaining();

 private:
  std::unique_ptr<BatchInfo> batch_info_;
};

/**
 * Row oriented batch of results.
 */
class RBatch : public Batch {
 public:
  RBatch(BatchInfo batch_info);

  virtual std::list<ColumnValue> pop();

  virtual BatchIterator iterator();

  virtual int NumBatchesRemaining();

 private:
  std::unique_ptr<BatchInfo> batch_info_;
};

}

#endif // HS2CLIENT_BATCH_H
