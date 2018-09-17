//
// Created by anyone on 18-8-24.
//

#ifndef BOXED_UNIQUE_PTR_WRAPPER_HPP
#define BOXED_UNIQUE_PTR_WRAPPER_HPP

#include <boost/hana.hpp>
#include <bits/unique_ptr.h>

template <typename INNER>
struct UNIQUE_PTR_WRAPPER{
    std::unique_ptr<INNER> p;
    UNIQUE_PTR_WRAPPER(INNER* _p):p(_p){};
    UNIQUE_PTR_WRAPPER(UNIQUE_PTR_WRAPPER<INNER>&& o){p=std::move(o.p);}
};

#endif //BOXED_UNIQUE_PTR_WRAPPER_HPP
