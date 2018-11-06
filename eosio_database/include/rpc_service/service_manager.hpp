//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <map>
#include <string>
#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>


namespace protobuf_rpc {

    using ::google::protobuf::Service;
    using ::google::protobuf::RpcController;

    class rpc_service;
    class rpc_method;
    class service_manager {
    public:
        service_manager() = default;
        ~service_manager() = default;

        void register_service(Service* service);
        void handle_rpc_call(const char *data,const size_t len,std::string &ret);

    private:
        inline std::shared_ptr<Service> get_service(const uint32_t id) const;
        inline const std::shared_ptr<rpc_service> get_rpc_service(const uint32_t id) const;
        inline const std::shared_ptr<rpc_method>  get_method(const uint32_t service_id,const uint32_t method_id) const;

    private:
        std::map<uint32_t,std::shared_ptr<rpc_service>> _services;
    };

}