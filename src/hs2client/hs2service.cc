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

namespace hs2client {

struct HS2Service::Impl {
  boost::shared_ptr<TSocket> socket_;
  boost::shared_ptr<TTransport> transport_;
  boost::shared_ptr<TProtocol> protocol_;

  std::shared_ptr<TCLIServiceClient> client_;
};

Status HS2Service::Connect(const std::string& host, int port, int timeout, bool use_ssl,
    std::unique_ptr<HS2Service>* out) {
  std::unique_ptr<HS2Service::Impl> impl(new HS2Service::Impl());

  // TODO: error checking
  impl->socket_.reset(new TSocket(host, port));
  impl->transport_.reset(new TBufferedTransport(impl->socket_));
  impl->protocol_.reset(new TBinaryProtocol(impl->transport_));

  impl->client_.reset(new TCLIServiceClient(impl->protocol_));

  out->reset(new HS2Service(impl.release()));

  return Status::Success();
}

HS2Service::HS2Service(HS2Service::Impl* impl): impl_(impl) {}

HS2Service::~HS2Service() = default;

Status HS2Service::OpenSession(const std::string& user, const HS2ClientConfig& config,
    std::unique_ptr<HS2Session>* out) {
  out->reset(new HS2Session(this));

  return Status::Success();
}

}
