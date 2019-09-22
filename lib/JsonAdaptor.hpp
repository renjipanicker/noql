#pragma once
#include "Module.hpp"
#include "noql/Adaptor.hpp"

namespace noq {
    class JsonAdaptor : public noq::Adaptor {
    public:
        struct JsonFactory : public noq::Factory {
            std::unique_ptr<Adaptor> create(const noq::Url& url) const override;
        };

        using JsonValue = nlohmann::json;
    private:
        mutable JsonValue root_;
        std::string idName_;
    public:
        inline JsonAdaptor() {}
        inline JsonAdaptor(std::istream& is) {
            loadStream(is);
        }

        void loadStream(std::istream& is);
        void saveStream(std::ostream& os);

        void insert(const std::string& obj) const override;
        std::vector<noq::Value> queryValueByKey(const noq::Value& key) const override;
        std::vector<noq::Value> queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const override;
        std::vector<noq::Value> queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const override;
    };
}
