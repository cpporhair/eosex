//
// Created by 杨文宇 on 2018/10/25.
//
#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template <typename T>
class queue {
public:
    queue(){}
    queue(const queue &q) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = q._queue;
    }
    void push(T &v) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(v);
        _cond.notify_all();
    }
    void wait_and_pop(T &v) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock,[this]{ return !_queue.empty(); });
        v = _queue.front();
        _queue.pop();
    }
    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock,[&]{ return !_queue.empty(); });
        auto res = _queue.front();
        _queue.pop();
        return res;
    }
    bool try_pop(T &v) {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_queue.empty()) return false;

        v = _queue.front();
        _queue.pop();
        return true;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }
private:
    mutable std::mutex      _mutex;
    std::queue<T>           _queue;
    std::condition_variable _cond;
};
