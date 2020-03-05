//
// Created by 杨文宇 on 2018/11/9.
//
#pragma once

#include <memory>
#include <utility>

class function_wrapper {
public:
    template <typename F>
    function_wrapper(F &&f):_impl{new impl_type<F>(std::move(f))}{}
    function_wrapper() = default;
    function_wrapper(function_wrapper &&other): _impl(std::move(other._impl)){}
    function_wrapper& operator=(function_wrapper &&other) {
        _impl = std::move(other._impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
    void operator()() {
        _impl->call();
    }
private:
    struct impl_base {
        virtual ~impl_base(){}

        virtual void call = 0;
    };

    template <typename F>
    struct impl_type : impl_base {
        F _f;
        impl_type(F &&f): _f(std::move(f)){}
        virtual void call() {
            f();
        }
    };

private:
    std::unique_ptr<impl_base> _impl;

};
