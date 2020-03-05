//
// Created by 杨文宇 on 2018/10/25.
//

#include <memory>
#include <exception>
#include <rpc_service/service_manager.hpp>
#include <rpc_service/rpc_method.hpp>
#include <rpc_service/rpc_service.hpp>
#include <rpc_service/rpc_controller.hpp>
#include <rpc_service/utility.hpp>
#include <protocol/message.pb.h>

namespace protobuf_rpc {

    using action_message::rpc_request;
    using action_message::rpc_response;

    void service_manager::register_service(Service* service) {
        const ServiceDescriptor *service_desc = service->GetDescriptor();
        uint32_t id = djb_hash(service_desc->full_name());
        if(_services.find(id) != _services.end())
            throw std::logic_error("duplicated service id");

        _services[id] = std::make_shared<rpc_service>(service);
    }

    void service_manager::handle_rpc_call(const char *data, const size_t len, std::string &ret) {

        std::shared_ptr<rpc_controller> controller{new rpc_controller};
        rpc_request req;
        req.ParseFromArray(data,len);
        uint32_t service_id = req.head().service_id();
        uint32_t method_id = req.head().method_id();
        const std::shared_ptr<Service> service = get_service(service_id);
        const std::shared_ptr<rpc_method> method = get_method(service_id,method_id);

        std::shared_ptr<Message> req_param{ method->request_msg()->New() };
        std::shared_ptr<Message> resp_param{ method->response_msg()->New() };
        req_param->ParseFromString(req.data());
        service->CallMethod(method->descriptor(),controller.get(),req_param.get(),resp_param.get(), nullptr);

        rpc_response resp;
        resp.set_data(resp_param->SerializeAsString());
        resp.SerializeToString(&ret);
    }

    std::shared_ptr<Service> service_manager::get_service(const uint32_t id) const {
        return get_rpc_service(id)->service();
    }

    const std::shared_ptr<rpc_service> service_manager::get_rpc_service(const uint32_t id) const {
        return _services.at(id);
    }

    const std::shared_ptr<rpc_method> service_manager::get_method(const uint32_t service_id, const uint32_t method_id) const {
        return get_rpc_service(service_id)->get_method(method_id);
    }

}
