//
// Created by 杨文宇 on 2018/10/25.
//

#pragma once

#include <string>
#include <google/protobuf/service.h>

namespace protobuf_rpc {
    using ::google::protobuf::RpcController;
    using ::google::protobuf::Closure;

    class rpc_controller : public RpcController {
    public:

        rpc_controller() {
            Reset();
        }

        virtual ~rpc_controller(){}

        virtual void Reset() {
            _failed = false;
        }

        virtual bool Failed() const {
            return _failed;
        }

        virtual std::string ErrorText() const {
            return _error_text;
        }

        virtual void StartCancel(){}

        virtual void SetFailed(const std::string& reason) {
            _failed = true;
            _error_text = reason;
        }

        void append_failed( const std::string &reason ) {
            _error_text += reason;
        }

        template<typename Error>
        Error get_error() const {
            Error error;
            error.set_error_text(_error_text);
            return error;
        }

        virtual bool IsCanceled() const {
            return false;
        }

        virtual void NotifyOnCancel(Closure* callback) {}

    private:
        bool        _failed;
        std::string _error_text;
    };
}
