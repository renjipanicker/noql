#include "pch.hpp"
#include "Module.hpp"
#include "noql/Connection.hpp"

int noq::Parameter::compare(const Parameter& rhs) const {
    if(type_ != rhs.type_){
        if(type_ == Type::Value){
            return +1;
        }
        return -1;
    }
    assert(type_ == rhs.type_);
    if (value_ < rhs.value_) {
        return -1;
    }
    if (value_ > rhs.value_) {
        return +1;
    }
    return 0;
}

int noq::Function::compare(const Function& rhs) const {
    if(ns_ < rhs.ns_) {
        return -1;
    }
    if(ns_ > rhs.ns_) {
        return +1;
    }
    if(name_ < rhs.name_) {
        return -1;
    }
    if(name_ > rhs.name_) {
        return +1;
    }
    if(arity() < rhs.arity()) {
        return -1;
    }
    if(arity() > rhs.arity()) {
        return +1;
    }
    assert(arity() == rhs.arity());
    auto lit = parameterList().begin();
    auto lite = parameterList().end();
    auto rit = rhs.parameterList().begin();
    auto rite = rhs.parameterList().end();
    while((lit != lite) && (rit != rite)){
        auto pc = lit->compare(*rit);
        if(pc != 0){
            return pc;
        }
        ++lit;
        ++rit;
    }
    return 0;
}
