//
// Created by 杨文宇 on 2018/11/9.
//
#pragma once

#include <atomic>
#include <thread>
#include <functional>
#include <vector>
#include <thread_safe/queue.hpp>
#include "join_threads.hpp"

class thread_pool {
public:
    thread_pool():_done{false},_joiner{_threads} {
        const unsigned count = std::thread::hardware_concurrency();
        try {

            for(unsigned i = 0;i < count;++i) {
                _threads.push_back(std::thread(&thread_pool::worker_thread,this));
            }

        } catch (...) {
            _done = true;
            throw;
        }
    }

    ~thread_pool() {
        _done = true;
    }
    template <typename FuncType>
    void submit(FuncType f) {
        _work_queue.push(std::function<void()>>(f));
    }
private:
    void worker_thread () {
        while (!_done) {
            std::function<void()> task;
            if(_work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }
private:
    std::atomic_bool _done;
    queue<std::function<void()>> _work_queue;
    std::vector<std::thread> _threads;
    join_threads _joiner;
};
