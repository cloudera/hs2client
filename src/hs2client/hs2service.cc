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

#include "hs2client/hs2service.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/TCLIService.h"
#include "hs2client/hs2session.h"

using apache::hive::service::cli::thrift::TCLIServiceClient;
using apache::thrift::protocol::TBinaryProtocol;
using apache::thrift::protocol::TProtocol;
using apache::thrift::transport::TBufferedTransport;
using apache::thrift::transport::TSocket;
using apache::thrift::transport::TTransport;
using namespace std;

namespace hs2client {

struct HS2Service::HS2ServiceImpl {
  // The use of boost here is required for Thrift compatibility.
  boost::shared_ptr<TSocket> socket_;
  boost::shared_ptr<TTransport> transport_;
  boost::shared_ptr<TProtocol> protocol_;

  shared_ptr<TCLIServiceClient> client_;
};

Status HS2Service::Connect(const string& host, int port, int timeout, bool use_ssl,
    unique_ptr<HS2Service>* out) {
  out->reset(new HS2Service(host, port, timeout, use_ssl));

  return (*out)->Open();
}

HS2Service::HS2Service(const string& host, int port, int timeout, bool use_ssl)
    : host_(host), port_(port), timeout_(timeout), use_ssl_(use_ssl),
      impl_(new HS2ServiceImpl()) {}

HS2Service::~HS2Service() = default;

Status HS2Service::Open() {
  // TODO: error checking
  impl_->socket_.reset(new TSocket(host_, port_));
  impl_->transport_.reset(new TBufferedTransport(impl_->socket_));
  impl_->protocol_.reset(new TBinaryProtocol(impl_->transport_));

  impl_->client_.reset(new TCLIServiceClient(impl_->protocol_));

  return Status::Success();
}

Status HS2Service::OpenSession(const string& user, const HS2ClientConfig& config,
    unique_ptr<HS2Session>* out) {
  out->reset(new HS2Session(this));

  return (*out)->Open();
}

}
