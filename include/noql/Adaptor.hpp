#pragma once

#include "Url.hpp"

namespace noq {
    class Adaptor {
    public:
        virtual ~Adaptor(){}
        virtual void insert(const std::string& obj) const = 0;
        virtual std::vector<noq::Value> queryValueByKey(const noq::Value& key) const = 0;
        virtual std::vector<noq::Value> queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const = 0;
        virtual std::vector<noq::Value> queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const = 0;
    };

    class Factory {
    public:
        virtual ~Factory(){}
        virtual std::unique_ptr<Adaptor> create(const noq::Url& url) const = 0;
    };
}
