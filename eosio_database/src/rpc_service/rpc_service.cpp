//
// Created by 杨文宇 on 2018/10/25.
//
#include <exception>
#include <utility>
#include <rpc_service/utility.hpp>
#include <rpc_service/rpc_service.hpp>

namespace protobuf_rpc {

    rpc_service::rpc_service():_service{nullptr} {}

    rpc_service::rpc_service(Service *service):_service{service} {
        init();
    }

    void rpc_service::add_method(const uint32_t id,std::shared_ptr<rpc_method> method) {
        if( _methods.find(id) != _methods.end() )
            throw std::logic_error("duplicated method id");

        _methods[id] = method;
    }

    void rpc_service::init() {
        const ServiceDescriptor *service_desc = _service->GetDescriptor();
        const MethodDescriptor  *method_desc{nullptr};
        const Message           *request{nullptr};
        const Message           *response{nullptr};

        for( int i = 0;i < service_desc->method_count();++i ) {
            method_desc = service_desc->method(i);
            request = &_service->GetRequestPrototype(method_desc);
            response = &_service->GetResponsePrototype(method_desc);
            uint32_t id = djb_hash(method_desc->full_name());
            std::shared_ptr<rpc_method> method{new rpc_method{method_desc,request,response,id}};
            add_method(id,method);
        }
    }

    const std::shared_ptr<rpc_method> rpc_service::get_method(const uint32_t id) const {

        if(_service == nullptr)
            throw std::runtime_error("service not exist");

        return _methods.at(id);
    }
}

