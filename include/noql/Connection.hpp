#pragma once

#include "Value.hpp"
#include "Function.hpp"
#include "Adaptor.hpp"

namespace noq {
    class Rule;
    class Connection {
        struct Impl;
        std::unique_ptr<Impl> impl_;

        void compileStreamModule(std::istream& is);
    public:
        Connection();
        Connection(const std::string& str);
        ~Connection();

        std::function<void(const bool&, const std::string&, const std::vector<Function>&, const std::vector<Function>&)> testCB_;

        static void registerAdaptor(const std::string& name, std::unique_ptr<noq::Factory>&& factory);

        bool hasRuleList(const std::string& ns) const;
        const std::vector<Rule>& ruleList(const std::string& ns) const;

        void open(const std::string& str);
        void close();
        void exec(std::istream& ss) const;
        void execString(const std::string& obj) const;
        void execFile(const std::string& filename) const;

        void insert(const std::string& obj) const;
        std::vector<noq::Value> queryValueByKey(const noq::Value& key) const;
        std::vector<noq::Value> queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const;
        std::vector<noq::Value> queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const;
    };

    void init();
}
