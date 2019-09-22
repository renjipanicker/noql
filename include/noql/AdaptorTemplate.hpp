#pragma once
#include "Module.hpp"

// This file is not used anywhere,it is to be copy-pasted to wherever a new adaptor should be used.
namespace foo {
    class FooAdaptor : public noq::Adaptor {
    public:
        void insert(const std::string& obj) const override {
        }

        std::vector<noq::Value> queryValueByKey(const noq::Value& key) const override {
            return std::vector<noq::Value>();
        }

        std::vector<noq::Value> queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const override {
            return std::vector<noq::Value>();
        }

        std::vector<noq::Value> queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const override {
            return std::vector<noq::Value>();
        }
    };

}