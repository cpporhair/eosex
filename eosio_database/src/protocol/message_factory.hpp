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

    template <typename T>
    static T* produce(int32_t key) {
        if(_map.find(key) == _map.end())
            throw std::invalid_argument("the message is not exist!");

        return dynamic_cast<T*>(_map[key]());
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

std::map<int32_t,std::function<::google::protobuf::Message*()>> message_factory::_map;

#define REGISTER_MESSAGE_NAME(T) reg_msg_##T
#define REGISTER_MESSAGE(Key1,Key2,T,...) static message_factory::register_t<T> REGISTER_MESSAGE_NAME(T)(Key1,Key2,##__VA_ARGS__);

using namespace action_message;

REGISTER_MESSAGE(db_store_i64_request,db_store_i64_response,db_store_i64)
REGISTER_MESSAGE(db_update_i64_request,db_update_i64_response,db_update_i64)
REGISTER_MESSAGE(db_remove_i64_request,db_remove_i64_response,db_remove_i64)
REGISTER_MESSAGE(db_get_i64_request,db_get_i64_response,db_get_i64)
REGISTER_MESSAGE(db_next_i64_request,db_next_i64_response,db_next_64)
REGISTER_MESSAGE(db_previous_i64_request,db_previous_i64_response,db_previous_i64)
REGISTER_MESSAGE(db_find_i64_request,db_find_i64_response,db_find_i64)
REGISTER_MESSAGE(db_lowerbound_i64_request,db_lowerbound_i64_response,db_lowerbound_i64)
REGISTER_MESSAGE(db_upperbound_i64_request,db_upperbound_i64_response,db_upperbound_i64)
REGISTER_MESSAGE(db_end_i64_request,db_end_i64_response,db_end_i64)

REGISTER_MESSAGE(db_idx64_store_request,db_idx64_store_response,db_idx64_store)
REGISTER_MESSAGE(db_idx64_update_request,db_idx64_update_response,db_idx64_update)
REGISTER_MESSAGE(db_idx64_remove_request,db_idx64_remove_response,db_idx64_remove)
REGISTER_MESSAGE(db_idx64_find_secondary_request,db_idx64_find_secondary_response,db_idx64_find_secondary)
REGISTER_MESSAGE(db_idx64_find_primary_request,db_idx64_find_primary_response,db_idx64_find_primary)
REGISTER_MESSAGE(db_idx64_lowerbound_request,db_idx64_lowerbound_response,db_idx64_lowerbound)
REGISTER_MESSAGE(db_idx64_upperbound_request,db_idx64_upperbound_response,db_idx64_upperbound)
REGISTER_MESSAGE(db_idx64_end_request,db_idx64_end_response,db_idx64_end)
REGISTER_MESSAGE(db_idx64_next_request,db_idx64_next_response,db_idx64_next)
REGISTER_MESSAGE(db_idx64_previous_request,db_idx64_previous_response,db_idx64_previous)

REGISTER_MESSAGE(db_idx128_store_request,db_idx128_store_response,db_idx128_store)
REGISTER_MESSAGE(db_idx128_update_request,db_idx128_update_response,db_idx128_update)
REGISTER_MESSAGE(db_idx128_remove_request,db_idx128_remove_response,db_idx128_remove)
REGISTER_MESSAGE(db_idx128_find_secondary_request,db_idx128_find_secondary_response,db_idx128_find_secondary)
REGISTER_MESSAGE(db_idx128_find_primary_request,db_idx128_find_primary_response,db_idx128_find_primary)
REGISTER_MESSAGE(db_idx128_lowerbound_request,db_idx128_lowerbound_response,db_idx128_lowerbound)
REGISTER_MESSAGE(db_idx128_upperbound_request,db_idx128_upperbound_response,db_idx128_upperbound)
REGISTER_MESSAGE(db_idx128_end_request,db_idx128_end_response,db_idx128_end)
REGISTER_MESSAGE(db_idx128_next_request,db_idx128_next_response,db_idx128_next)
REGISTER_MESSAGE(db_idx128_previous_request,db_idx128_previous_response,db_idx128_previous)

REGISTER_MESSAGE(db_idx256_store_request,db_idx256_store_response,db_idx256_store)
REGISTER_MESSAGE(db_idx256_update_request,db_idx256_update_response,db_idx256_update)
REGISTER_MESSAGE(db_idx256_remove_request,db_idx256_remove_response,db_idx256_remove)
REGISTER_MESSAGE(db_idx256_find_secondary_request,db_idx256_find_secondary_response,db_idx256_find_secondary)
REGISTER_MESSAGE(db_idx256_find_primary_request,db_idx256_find_primary_response,db_idx256_find_primary)
REGISTER_MESSAGE(db_idx256_lowerbound_request,db_idx256_lowerbound_response,db_idx256_lowerbound)
REGISTER_MESSAGE(db_idx256_upperbound_request,db_idx256_upperbound_response,db_idx256_upperbound)
REGISTER_MESSAGE(db_idx256_end_request,db_idx256_end_response,db_idx256_end)
REGISTER_MESSAGE(db_idx256_next_request,db_idx256_next_response,db_idx256_next)
REGISTER_MESSAGE(db_idx256_previous_request,db_idx256_previous_response,db_idx256_previous)

REGISTER_MESSAGE(db_idx_double_store_request,db_idx_double_store_response,db_idx_double_store)
REGISTER_MESSAGE(db_idx_double_update_request,db_idx_double_update_response,db_idx_double_update)
REGISTER_MESSAGE(db_idx_double_remove_request,db_idx_double_remove_response,db_idx_double_remove)
REGISTER_MESSAGE(db_idx_double_find_secondary_request,db_idx_double_find_secondary_response,db_idx_double_find_secondary)
REGISTER_MESSAGE(db_idx_double_find_primary_request,db_idx_double_find_primary_response,db_idx_double_find_primary)
REGISTER_MESSAGE(db_idx_double_lowerbound_request,db_idx_double_lowerbound_response,db_idx_double_lowerbound)
REGISTER_MESSAGE(db_idx_double_upperbound_request,db_idx_double_upperbound_response,db_idx_double_upperbound)
REGISTER_MESSAGE(db_idx_double_end_request,db_idx_double_end_response,db_idx_double_end)
REGISTER_MESSAGE(db_idx_double_next_request,db_idx_double_next_response,db_idx_double_next)
REGISTER_MESSAGE(db_idx_double_previous_request,db_idx_double_previous_response,db_idx_double_previous)

REGISTER_MESSAGE(db_idx_long_double_store_request,db_idx_long_double_store_response,db_idx_long_double_store)
REGISTER_MESSAGE(db_idx_long_double_update_request,db_idx_long_double_update_response,db_idx_long_double_update)
REGISTER_MESSAGE(db_idx_long_double_remove_request,db_idx_long_double_remove_response,db_idx_long_double_remove)
REGISTER_MESSAGE(db_idx_long_double_find_secondary_request,db_idx_long_double_find_secondary_response,db_idx_long_double_find_secondary)
REGISTER_MESSAGE(db_idx_long_double_find_primary_request,db_idx_long_double_find_primary_response,db_idx_long_double_find_primary)
REGISTER_MESSAGE(db_idx_long_double_lowerbound_request,db_idx_long_double_lowerbound_response,db_idx_long_double_lowerbound)
REGISTER_MESSAGE(db_idx_long_double_upperbound_request,db_idx_long_double_upperbound_response,db_idx_long_double_upperbound)
REGISTER_MESSAGE(db_idx_long_double_end_request,db_idx_long_double_end_response,db_idx_long_double_end)
REGISTER_MESSAGE(db_idx_long_double_next_request,db_idx_long_double_next_response,db_idx_long_double_next)
REGISTER_MESSAGE(db_idx_long_double_previous_request,db_idx_long_double_previous_response,db_idx_long_double_previous)
