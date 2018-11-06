//
// Created by 杨文宇 on 2018/10/26.
//
#pragma once

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <network/tcp_client.hpp>

namespace protobuf_rpc {

    using ::google::protobuf::ServiceDescriptor;
    using ::google::protobuf::MethodDescriptor;
    using ::google::protobuf::RpcChannel;
    using ::google::protobuf::Message;

    class rpc_channel : public RpcChannel {
    public:
        rpc_channel(const std::string &remote,uint32_t port);
        ~rpc_channel();

        virtual void CallMethod(const MethodDescriptor* method,
                                RpcController* controller,
                                const Message* request,
                                Message* response,
                                Closure* done)；
    private:
        tcp_client _client;
    };

}
