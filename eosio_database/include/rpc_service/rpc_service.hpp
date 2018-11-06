//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <map>
#include <string>
#include <memory>
#include <rpc_service/rpc_method.hpp>

namespace protobuf_rpc {

    using ::google::protobuf::ServiceDescriptor;
    using ::google::protobuf::Service;

    class rpc_service {
    public:
        rpc_service();
        rpc_service(Service *service);

        void add_method( const uint32_t id,std::shared_ptr<rpc_method> method );
        const std::shared_ptr<rpc_method> get_method( const uint32_t id) const;
        const uint32_t id() const {
            return _id;
        }
        const std::shared_ptr<Service> service() const {
            return _service;
        }
    private:
        void init();

    private:
        std::shared_ptr<Service> _service;
        std::map<uint32_t ,std::shared_ptr<rpc_method>> _methods;
        uint32_t _id;
    };

}