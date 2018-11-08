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
    template <typename T>
    struct register_t {
        register_t(int32_t key1,int32_t key2) {
            message_factory::get()._map.emplace(key1,[]{ return new T(); });
            message_factory::get()._map.emplace(key2,[]{ return new T(); });
        }
        template <typename... Args>
        register_t(int32_t key1,int32_t key2,Args... args) {
            message_factory::get()._map.emplace(key1,[&]{return new T(args...);});
            message_factory::get()._map.emplace(key2,[&]{return new T(args...);});
        }
    };


    static ::google::protobuf::Message* create(int32_t key) {
        if(_map.find(key) == _map.end())
            throw std::invalid_argument("the message is not exist!");

        return _map[key]();
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



#define REGISTER_MESSAGE_NAME(T) reg_msg_##T
#define REGISTER_MESSAGE(Key1,Key2,T,...) static message_factory::register_t<T> REGISTER_MESSAGE_NAME(T)(Key1,Key2,##__VA_ARGS__);
