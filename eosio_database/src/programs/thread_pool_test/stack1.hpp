//
// Created by 杨文宇 on 2018/11/14.
//
#pragma once

#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <stack>

template <typename T>
class stack_v1 {
public:
    void push(const T &elem) {
        elems_.push_back(elem);
    }
    void pop() {
        assert(!elems_.empty());
        elems_.pop_back();
    }
    const T& top() const {
        assert(!elems_.empty());
        return elems_.back();
    }
    bool empty() const {
        return elems_.empty();
    }

    void print_on(std::ostream &out) const {
        for(const T& elem : elems_)
            out << elem << ' ';
    }

    friend std::ostream& operator <<(std::ostream &out,const stack_v1<T> &s) {
        s.print_on(out);
        return out;
    }

private:
    std::vector<T> elems_;
};

template <typename T>
class stack_v1<T*> {
public:
private:

};

template <>
class stack_v1<std::string> {
public:
    void push(const std::string&);
    void pop();
    const std::string& top() const;
    bool empty() const;
private:
    std::stack<std::string> elems_;
};

void stack_v1<std::string>::push(const std::string &elem) {
    elems_.push(elem);
}

void stack_v1<std::string>::pop() {
    assert(!elems_.empty());
    elems_.pop();
}

const std::string& stack_v1<std::string>::top() const {
    assert(!elems_.empty());
    return elems_.top();
}

bool stack_v1<std::string>::empty() const {
    return elems_.empty();
}

template <typename T,typename Cont = std::vector<T>>
class stack_v2 {
public:
    void push(const T &elem);
    void pop();
    const T& top() const;
    bool empty() const {
        return elems_.empty();
    }
private:
    Cont elems_;
};

template <typename T,typename Cont>
void stack_v2<T,Cont>::push(const T &elem) {
    elems_.push_back(elem);
}

template <typename T,typename Cont>
const T& stack_v2<T,Cont>::top() const {
    assert(!elems_.empty());
    return elems_.back();
}

template <typename T,typename Cont>
void stack_v2<T,Cont>::pop() {
    assert(!elems_.empty());
    elems_.pop_back();
}

template <typename T>
class stack_v3 {
public:
    stack_v3() = default;
    stack_v3(const T &elem):elems_({elem}){}
    void push(const T &elem) {
        elems_.push_back(elem);
    }
    const T& top() const {
        assert(!elems_.empty());
        return elems_.back();
    }
    void pop() {
        assert(!elems_.empty());
        elems_.pop_back();
    }
private:
    std::vector<T> elems_;
};
