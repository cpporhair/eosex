//
// Created by 杨文宇 on 2018/11/9.
//
#pragma once

#include <deque>
#include <mutex>
#include "function_wrapper.hpp"

class work_stealing_queue {
public:
    typedef function_wrapper data_type;

    work_stealing_queue(){}
    work_stealing_queue(const work_stealing_queue&) = delete;
    work_stealing_queue& operator=(const work_stealing_queue&) = delete;
    void push(data_type data) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push_front(std::move(data));
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }
    bool try_pop(data_type &res) {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty()) return false;
        res = std::move(_queue.front());
        _queue.pop_front();
        return true;
    }
    bool try_steal(data_type& res) {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty()) return false;
        res = std::move(_queue.back());
        _queue.pop_back();
        return true;
    }
private:
    std::deque<data_type> _queue;
    mutable std::mutex    _mutex;
};