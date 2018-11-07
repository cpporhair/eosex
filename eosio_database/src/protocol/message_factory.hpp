//
// Created by 杨文宇 on 2018/11/7.
//
#pragma once

#include <map>
#include <functional>
#include <memory>
#include <cstdint>
#include <protocol/message.pb.h>

class message_factory {
public:
    template <int32_t ID,typename T>
    struct register_t {
        register_t() {
            message_factory::get()._map.emplace(ID,[]{ return new T(); });
        }
        template <typename... Args>
        register_t(Args... args) {
            message_factory::get()._map.emplace(ID,[]{return new T(args...);});
        }
    };

    template <int32_t ID,typename T>
    static T* produce() {
        if(_map.find(ID) == _map.end())
            throw std::invalid_argument("the message is not exist!");

        return dynamic_cast<T*>(_map[ID]());
    }

private:
    message_factory(){}
    message_factory(const message_factory&) = delete;
    message_factory(message_factory&&) = delete;
    static message_factory& get() {
        static message_factory inst;
        return inst;
    }
    static std::map<int32_t,std::function<::google::protobuf::Message*()>> _map;
};

#define REGISTER_MESSAGE_NAME(T) reg_msg_##T##
#define REGISTER_MESSAGE(ID,T,...) static message_factory::register_t<ID,T> REGISTER_MESSAGE_NAME(T)(##__VA_ARGS__);

using namespace action_message;

REGISTER_MESSAGE(db_store_i64_request,db_stroe_i64)
REGISTER_MESSAGE(db_update_i64_request,db_update_i64)
REGISTER_MESSAGE(db_remove_i64_request,db_remove_i64)
REGISTER_MESSAGE(db_get_i64_request,db_get_i64)
REGISTER_MESSAGE(db_next_i64_request,db_next_64)
REGISTER_MESSAGE(db_previous_request,db_pervious_i64)
REGISTER_MESSAGE(db_find_i64)
REGISTER_MESSAGE(db_end_i64_request,db_end_i64)
