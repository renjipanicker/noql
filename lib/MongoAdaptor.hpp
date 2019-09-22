#pragma once
#include "Module.hpp"
#include "noql/Adaptor.hpp"

namespace noq {
    class MongoAdaptor : public noq::Adaptor {
        struct Impl;
        std::unique_ptr<Impl> impl_;
    public:
        struct MongoFactory : public noq::Factory {
            MongoFactory();
            ~MongoFactory();
            std::unique_ptr<Adaptor> create(const noq::Url& url) const override;
        };

        inline MongoAdaptor();
        inline ~MongoAdaptor();
        inline MongoAdaptor(const noq::Url& url);

        void open(const noq::Url& url);
        void close();

        void insert(const std::string& obj) const override;
        std::vector<noq::Value> queryValueByKey(const noq::Value& key) const override;
        std::vector<noq::Value> queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const override;
        std::vector<noq::Value> queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const override;
    };
}
