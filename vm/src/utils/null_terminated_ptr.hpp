//
// Created by null on 18-8-30.
//

#ifndef BOXED_NULL_TERMINATED_PTR_HPP
#define BOXED_NULL_TERMINATED_PTR_HPP

#include <type_traits>
struct null_terminated_ptr {
    explicit null_terminated_ptr(char* value) : value(value) {
    }
    typename std::add_lvalue_reference<char>::type operator*() const {
        return *value;
    }
    char *operator->() const noexcept {
        return value;
    }
    template<typename U>
    operator U *() const {
        return static_cast<U *>(value);
    }
    char *value;
};
#endif //BOXED_NULL_TERMINATED_PTR_HPP
