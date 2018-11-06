//
// Created by 杨文宇 on 2018/10/26.
//

#include <cstring>
#include <rpc_service/rpc_channel.hpp>
#include <rpc_service/utility.hpp>
#include <protocol/message.pb.h>

namespace protobuf_rpc {

    rpc_channel::rpc_channel(const std::string &remote, uint32_t port):_client{remote,port} {
        _client.connect();
    }

    rpc_channel::~rpc_channel() {
        _client.close();
    }

    void rpc_channel::CallMethod(const google::protobuf::MethodDescriptor *method, RpcController *controller,
                                 const google::protobuf::Message *request, google::protobuf::Message *response,
                                 Closure *done) {

        const ServiceDescriptor* service_desc = method->service();
        uint32_t service_id = djb_hash(service_desc->full_name());
        uint32_t method_id = djb_hash(method->full_name());
        rpc_request req;
        req.mutable_head()->set_service_id(service_id);
        req.mutable_head()->set_method_id(method_id);
        req.mutable_data()->set_data(request->SerializeAsString());
        uint32_t len = req.ByteSize();
        uint32_t buf_size = sizeof(len) + len;
        char *buf = new char[buf_size];
        std::memcpy(buf,&len,sizeof(len));
        req.SerializeToArray(buf + sizeof(len),len);
        _client.send(buf,buf_size);
        char *ret_buf = new char[1024*1024];
        std::size_t ret_len = _client.receive(ret_buf,1024*1024);
        response->ParseFromArray(ret_buf,ret_len);
        delete []buf;
        delete []ret_buf;
    }

}

