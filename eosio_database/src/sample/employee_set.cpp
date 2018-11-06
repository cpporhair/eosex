//
// Created by 杨文宇 on 2018/10/19.
//
#include <sample/employee.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

using namespace boost::multi_index;

typedef multi_index_container<
    employee,
    indexed_by<
        ordered_unique<identity<employee>>,
        ordered_non_unique<mem_fun<employee,std::string&,employee::name>>
    >
> employee_set;

