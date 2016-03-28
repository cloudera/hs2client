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

#ifndef HS2CLIENT_LOGGING_H
#define HS2CLIENT_LOGGING_H

#include <iostream>

namespace hs2client {

// Stubbed versions of macros defined in glog/logging.h, intended for
// environments where glog headers aren't available.
//
// Add more as needed.

// Log levels. LOG ignores them, so their values are abitrary.

#define HS2CLIENT_INFO 0
#define HS2CLIENT_WARNING 1
#define HS2CLIENT_ERROR 2
#define HS2CLIENT_FATAL 3

#define HS2CLIENT_LOG_INTERNAL(level) hs2client::internal::CerrLog(level)
#define HS2CLIENT_LOG(level) HS2CLIENT_LOG_INTERNAL(HS2CLIENT_##level)

#define HS2CLIENT_CHECK(condition)                                   \
  (condition) ? 0 : ::hs2client::internal::FatalLog(HS2CLIENT_FATAL) \
      << __FILE__ << __LINE__ << "Check failed: " #condition " "

#ifdef NDEBUG
#define HS2CLIENT_DFATAL HS2CLIENT_WARNING

#define DCHECK(condition) \
  while (false)           \
    hs2client::internal::NullLog()
#define DCHECK_EQ(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()
#define DCHECK_NE(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()
#define DCHECK_LE(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()
#define DCHECK_LT(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()
#define DCHECK_GE(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()
#define DCHECK_GT(val1, val2) \
  while (false)               \
    hs2client::internal::NullLog()

#else
#define HS2CLIENT_DFATAL HS2CLIENT_FATAL

#define DCHECK(condition) HS2CLIENT_CHECK(condition)
#define DCHECK_EQ(val1, val2) HS2CLIENT_CHECK((val1) == (val2))
#define DCHECK_NE(val1, val2) HS2CLIENT_CHECK((val1) != (val2))
#define DCHECK_LE(val1, val2) HS2CLIENT_CHECK((val1) <= (val2))
#define DCHECK_LT(val1, val2) HS2CLIENT_CHECK((val1) < (val2))
#define DCHECK_GE(val1, val2) HS2CLIENT_CHECK((val1) >= (val2))
#define DCHECK_GT(val1, val2) HS2CLIENT_CHECK((val1) > (val2))

#endif  // NDEBUG

namespace internal {

class NullLog {
 public:
  template <class T>
    NullLog& operator<<(const T& t) {
    return *this;
  }
};

class CerrLog {
 public:
  CerrLog(int severity)  // NOLINT(runtime/explicit)
    : severity_(severity),
      has_logged_(false) {}

  virtual ~CerrLog() {
    if (has_logged_) { std::cerr << std::endl; }
    if (severity_ == HS2CLIENT_FATAL) { std::exit(1); }
  }

  template <class T>
  CerrLog& operator<<(const T& t) {
    has_logged_ = true;
    std::cerr << t;
    return *this;
  }

  protected:
  const int severity_;
  bool has_logged_;
};

// Clang-tidy isn't smart enough to determine that DCHECK using CerrLog doesn't
// return so we create a new class to give it a hint.
class FatalLog : public CerrLog {
 public:
  FatalLog(int  severity )  // NOLINT
    : CerrLog(HS2CLIENT_FATAL) {}

  [[noreturn]] ~FatalLog() {
    if (has_logged_) { std::cerr << std::endl; }
    std::exit(1);
  }
};

}  // namespace internal

} // namespace hs2client

#endif  // HS2CLIENT_LOGGING_H
