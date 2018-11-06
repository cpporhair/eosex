//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

namespace protobuf_rpc {

    using ::google::protobuf::MethodDescriptor;
    using ::google::protobuf::Message;

    class rpc_method {
    public:
        rpc_method( const MethodDescriptor *descriptor,const Message *request,const Message *response,const uint32_t id):
            _descriptor{descriptor},
            _request{request},
            _response{response},
            _id{id}{
        }

        std::string get_method_info() const {

        }

        const MethodDescriptor* descriptor() const {
            return _descriptor;
        }

        const Message* request_msg() const {
            return _request;
        }

        const Message* response_msg() const {
            return _response;
        }

        const uint32_t id() const {
            return _id;
        }

    private:
      const MethodDescriptor *_descriptor;
      const Message          *_request;
      const Message          *_response;
      const uint32_t          _id;
    };
}
