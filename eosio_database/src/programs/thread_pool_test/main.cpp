//
// Created by 杨文宇 on 2018/11/9.
//
#include <iostream>
#include <functional>
#include <future>
#include <thread>
#include <exception>
#include <memory>
#include <boost/hana.hpp>
#include <string>
#include <boost/core/demangle.hpp>
#include <utility>
#include <type_traits>
#include <deque>
#include "stack1.hpp"


namespace hana = boost::hana;

void do_some_work() {
    std::cout << "to do something!" << std::endl;
}

void print_int(std::future<int>& future) {
    int x = future.get();
    std::cout << "value: " << x << std::endl;
}

class thread_interrupted : public std::exception {
public:
    virtual const char* what() const _NOEXCEPT {
        return "thread interrupted!";
    }
};

class interrupt_flag {
public:
    void set() {
        _flag = true;
    }
    bool is_set() const {
        return _flag;
    }

private:
    bool _flag{false};
};

class data {
public:
    data(int a,int b):_a{a},_b{b}{}
    int _a;
    int _b;
};

typedef std::shared_ptr<data> data_ptr;

class db {
public:

    db() {
        _data = std::shared_ptr<data>(new data{1,2} );
    }

    data_ptr& get_data() {
        return _data;
    }

    void run() {
        _data = std::shared_ptr<data>(new data{4,5} );
    }

private:
    data_ptr  _data;
};

class ctrl {
public:
    data_ptr _data;
};

thread_local interrupt_flag this_thread_interrupt_flag;

class interruptable_thread {
public:
    template <typename FuncType>
    interruptable_thread(FuncType f) {
        std::promise<interrupt_flag*> p;
        _internal_thread = std::thread([f,&p]{
            p.set_value(&this_thread_interrupt_flag);
            f();
        });
        _flag = p.get_future().get();
    }
    void interruption_point() {
        if(this_thread_interrupt_flag.is_set())
            throw thread_interrupted();
    }
    void interrupt() {
        if(_flag)
            _flag->set();
    }
private:
    std::thread    _internal_thread;
    interrupt_flag *_flag;
};

template <typename T>
bool equivalent( const T &a,const T &b ) {
    return !(a < b) && !(b < a);
}

template <typename T=int>
class big_number {
public:
    big_number(T a):_v{a}{}
    inline bool operator<(const big_number &b) const {
        return _v < b._v;
    }
private:
    T _v;
};

template <typename T1,typename T2>
class the_same_type {
public:
    enum {ret = false};
};

template <typename T>
class the_same_type<T,T> {
public:
    enum {ret = true};
};

template <typename T,int i>
class a_tmp{};

template<typename T,int i=1>
class some_computing {
public:
    typedef volatile T *ret_type;
    enum {
        ret_value = i + some_computing<T,i-1>::ret_value
    };
    static void f() {
        std::cout << "some_computing::i=" << i << '\n';
    }
};

template <typename T>
class some_computing<T,0> {
public:
    enum {ret_value = 0};
};

template<typename T>
class code_computing {
public:
    static void f() {
        T::f();
    }
};

template <int N>
class sum_int_set {
public:
    static const int re = N + sum_int_set<N-1>::re;
};

template <>
class sum_int_set<0> {
public:
    static const int re = 0;
};

template <bool c,typename Then,typename Else> class IF_{};
template <typename Then,typename Else>
class IF_<true,Then,Else>{
public:
    typedef Then reType;
};
template <typename Then,typename Else>
class IF_<false,Then,Else> {
public:
    typedef Else reType;
};

template<template<typename> class Condition, typename Statement>
class WHILE_ {
    template<typename Statement1>
    class STOP { public: typedef Statement1 reType; };
public:
    typedef typename
    IF_<Condition<Statement>::ret,
            WHILE_<Condition, typename Statement::Next>,
            STOP<Statement>>::reType::reType
            reType;
};


template<typename T>
class is_class {
private:
    typedef char one;
    typedef struct { char a[2]; } two;
    template<typename C> static one test(int C::*);
    template<typename C> static two test(...);
public:
    enum { yes = sizeof(is_class<T>::test<T>(0)) == 1 };
    enum { no = !yes };
};

class my_class{};

void call(my_class c) {
    std::cout << "my class" << std::endl;
}

void call(...) {
    std::cout << "..." << std::endl;
}

void call(float n) {
    std::cout << "float" << std::endl;
}

void call(int n) {
    std::cout << "int" << std::endl;
}

auto has_serialize = hana::is_valid( [](auto &&x) -> decltype(x.serialize()) {} );

/*template <typename T>
std::string serialize(const T &obj) {
    return hana::if_(has_serialize(obj),
                     [](auto &x){ return x.serialize(); },
                     [](auto &x){ return to_string(x); })(obj);
}*/

struct A{};

std::string to_string(const A &) {
    return "I am an A!";
}

struct B {
    std::string serialize() const {
        return "I am a B!";
    }
};

struct C {
    std::string serialize;
};

std::string to_string (const C &) {
    return "I am a C!";
}

struct D : A {
    std::string serialize() const {
        return "I am a D!";
    }
};

struct E {
    struct Functor {
        std::string operator()() {
            return "I am a E!";
        }
    };

    Functor serialize;
};

template <typename T,unsigned N>
std::size_t len(T(&)[N]) {
    return N;
}

template <typename T>
typename T::size_type len(T const &t) {
    return t.size();
}

std::size_t len(...) {
    return 0;
}

template <typename T>
struct has_serialize_v2 {
    typedef char yes[1];
    typedef char no[2];
    template <typename U,U u> struct really_has;

    template <typename C> static yes& test( really_has<std::string (C::*)(),&C::serialize>* ){}
    template <typename C> static yes& test( really_has<std::string (C::*)() const,&C::serialize>* ){}

    template <typename> static no& test(...){}

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

template <typename T>
bool test_has_serialize(const T&) {
    return has_serialize_v2<T>::value;
}

/*template <typename T>
std::string serialize(const T &obj) {
    if(test_has_serialize(obj)) {
        return obj.serialize();
    } else {
        return to_string(obj);
    }
}*/


template <typename T>
typename std::enable_if<has_serialize_v2<T>::value,std::string>::type serialize(const T &obj) {
    return obj.serialize();
}

template <typename T>
typename std::enable_if<!has_serialize_v2<T>::value,std::string>::type serialize(const T &obj) {
    return to_string(obj);
}

template <typename T1,typename T2>
auto max_v1(T1 a,T2 b) -> decltype(true?a:b) {
    return a > b ? a : b;
}

struct Default {
    int foo() const {return 1;}
};

struct NonDefault {
    NonDefault(const NonDefault&){}
    int foo() const {return 1;}
};

template <typename T1,typename T2,typename T3 = typename std::common_type<T1,T2>::type>
T3 max_v2(T1 a,T2 b) {
    return a > b ? a : b;
}

template <typename T1,typename T2,typename T3 = std::common_type_t<T1,T2>>
T3 max_v3(T1 a,T2 b) {
    return a > b ? a : b;
}

template <typename T1,typename T2>
auto max_v4(T1 a,T2 b) {
    return a > b ? a : b;
}


template <typename T>
using DequeStack = stack_v2<T,std::deque<T>>;


template <typename T>
T max(T a,T b) {
    std::cout << boost::core::demangle(typeid(a).name()) << std::endl;
    return a > b ? a : b;
}

template<typename T>
using EnableIfString = std::enable_if_t<std::is_convertible_v<T,std::string>>;

template <typename T>
using EnableIfString1 = std::enable_if_t<std::is_convertible<T,std::string>::value>;

class person {
public:
    template <typename STR,typename = EnableIfString1<STR>>
    explicit person(STR &&n):_name(std::forward<STR>(n)) {
        std::cout << "TMPL-CONSTR for '" << _name << "'\n";
    }
    person(const person &p):_name(p._name) {
        std::cout << "COPY-CONSTR person '" << _name << "'\n";
    }
    person(person &&p):_name(std::move(p._name)) {
        std::cout << "MOVE-CONSTR person '" << _name << "'\n";
    }
private:
    std::string _name;
};

template <typename T>
using EnableIfSizeGreater4 = std::enable_if_t<(sizeof(T) > 4)>;

template <typename T>
EnableIfSizeGreater4<T> foo() {
    std::cout << "foo()" << std::endl;
}

template <typename T>
class my_string {
public:
    my_string(){}
    my_string(const T *s) {}
};

template <typename T>
my_string<T> truncate_v1(my_string<T> const &str,int n) {
    return my_string<T>{};
}

int main(int argc,char **argv) {

    my_string<char> str1,str2;
    str1 = truncate_v1<char>("Hello World",5);

    foo<double>();

    std::string s = "sname";
    person p1(s);
    person p2("tmp");
    person p3(p1);
    person p4(std::move(p1));


    int x = 1;
    int y = 2;
    int &rx = x;
    int &ry = y;

    auto rre = max(rx,ry);

    std::cout << boost::core::demangle(typeid("bottom").name()) << std::endl;

    stack_v1<int>  int_stack;
    stack_v1<std::string> string_stack;

    int_stack.push(7);
    int_stack.push(8);
    int_stack.push(1);
    std::cout << int_stack << std::endl;


    string_stack.push("hello");
    string_stack.push("world");
    string_stack.pop();
    std::cout << string_stack.top() << std::endl;

    stack_v2<int> int_stack1;
    int_stack1.push(1);
    std::cout << int_stack1.top() << std::endl;

    DequeStack<int> deque_stack;
    deque_stack.push(3);
    std::cout << deque_stack.top() << std::endl;

    stack_v3 int_stack2 = 0;
    std::cout << int_stack2.top() << std::endl;

    auto ret1 = max_v1(3,7.2);
    std::cout << boost::core::demangle(typeid(ret1).name()) << std::endl;
    auto ret2 = max_v1(7,2.2);
    std::cout << boost::core::demangle(typeid(ret2).name()) << std::endl;

    auto ret3 = max_v2(7,2.2);
    std::cout << boost::core::demangle(typeid(ret3).name()) << std::endl;

    auto ret4 = max_v3(7,2.2);
    std::cout << boost::core::demangle(typeid(ret4).name()) << std::endl;

    auto ret5 = max_v4(7,2.2);
    std::cout << boost::core::demangle(typeid(ret5).name()) << std::endl;

    decltype(Default().foo()) n1 = 1;
    std::cout << boost::core::demangle(typeid(n1).name()) << std::endl;
    decltype(std::declval<NonDefault>().foo()) n2 = n1;
    std::cout << boost::core::demangle(typeid(n2).name()) << std::endl;

    std::enable_if<true,int>::type t1;
    std::cout << boost::core::demangle(typeid(t1).name()) << std::endl;
    std::enable_if<has_serialize_v2<B>::value,int>::type t2;
    std::cout << boost::core::demangle(typeid(t1).name()) << std::endl;



    A a;
    B b;
    C c;
    D d;

    std::cout << test_has_serialize(a) << std::endl;
    std::cout << test_has_serialize(b) << std::endl;
    std::cout << test_has_serialize(c) << std::endl;
    std::cout << test_has_serialize(d) << std::endl;


    std::cout << serialize(a) << std::endl;
    std::cout << serialize(b) << std::endl;
    std::cout << serialize(c) << std::endl;
    std::cout << serialize(d) << std::endl;

    decltype(b.serialize()) str;
    std::cout << boost::core::demangle(typeid(str).name()) << std::endl;

    std::cout << is_class<interruptable_thread>::yes << std::endl;

    const int len = 4;
    typedef
        IF_<sizeof(short)==len,short,
        IF_<sizeof(int)==len,int,
        IF_<sizeof(long)==len,long,
        IF_<sizeof(long long)==len,long long,
        void>::reType>::reType>::reType>::reType int_my;
    std::cout << sizeof(int_my) << '\n';

    std::cout << some_computing<int,500>::ret_value << '\n';
    code_computing<some_computing<int,99>>::f();

    std::cout << the_same_type<sum_int_set<1>,sum_int_set<2>>::ret << '\n';
    std::cout << sum_int_set<5>::re << '\n';

    std::cin.get();
    return 0;
}
