#include "pch.hpp"
#include "noql/Connection.hpp"
#include "noql/Query.hpp"
#include "noql/Url.hpp"
#include "JsonAdaptor.hpp"
#include "MongoAdaptor.hpp"
#include "Logger.hpp"

namespace {
    class NullAdaptor : public noq::Adaptor {
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

    std::map<const std::string, std::unique_ptr<noq::Factory>> s_factoryList;
}

struct noq::Connection::Impl {
    noq::Connection& conn_;
    std::unique_ptr<noq::Adaptor> adaptor_;
    struct RuleSet {
        std::vector<Rule> ruleList_;
    };
    std::map<std::string, RuleSet> ruleset_;

    inline auto hasRuleList(const std::string& ns) const {
        return (ruleset_.find(ns) != ruleset_.end());
    }

    inline auto& ruleList(const std::string& ns) const {
        return ruleset_.at(ns).ruleList_;
    }

    inline void open() {
        adaptor_ = std::make_unique<NullAdaptor>();
    }

    inline void open(const std::string& str) {
        auto url = noq::Url::parse(str);
        auto ait = s_factoryList.find(url.protocol);
        if(ait != s_factoryList.end()){
            auto adaptor = ait->second->create(url);
            adaptor_ = std::move(adaptor);
        }else{
            noq::Logger().warn("unknown adaptor:[{0}]", url.protocol);
        }
    }

    inline void close() {
    }

    inline Adaptor& getAdaptor() {
        return *adaptor_;
    }

    inline void insert(const std::string& obj) const {
        adaptor_->insert(obj);
    }

    inline auto queryValueByKey(const noq::Value& key) const {
        noq::Logger().trace("queryValueByObjectKey:key:[{1}]", 0, key);
        auto resl = adaptor_->queryValueByKey(key);
        return resl;
    }

    inline auto queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const {
        noq::Logger().trace("queryValueByObjectKey:obj:[{1}], key:[{2}]", 0, obj, key);
        auto resl = adaptor_->queryValueByObjectKey(obj, key);
        return resl;
    }

    inline auto queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const {
        noq::Logger().trace("queryValueByObjectKey:key:[{1}], val:[{2}]", 0, key, val);
        auto resl = adaptor_->queryObjectByKeyValue(key, val);
        return resl;
    }

    inline void verify(const noq::Verify& verify) {
        Query query;
        query.exec(verify.query_, conn_);
        auto& result = query.resl();
        if(result.size() != verify.result_.size()){
            conn_.testCB_(false, "result size mismatch", verify.result_, result);
            return;
        }
        auto rit = result.begin();
        auto rite = result.end();
        auto xit = verify.result_.begin();
        auto xite = verify.result_.end();
        while((rit != rite) && (xit != xite)){
            if(rit->compare(*xit) != 0){
                conn_.testCB_(false, "result mismatch", {*xit}, {*rit});
                return;
            }
            ++rit;
            ++xit;
        }
        conn_.testCB_(true, "", verify.result_, result);
    }

    struct StatementExec {
        Impl& impl_;
        inline StatementExec(Impl& impl) : impl_(impl) {}

        inline void operator()(const noq::Module::InsertDocumentStatement& stmt) {
            assert(stmt.fn.parameterList().size() == 1);
            auto& jstr = stmt.fn.parameterList().at(0).value().getString();
            impl_.insert(jstr);
        }
        inline void operator()(const noq::Module::InsertPairStatement& stmt) {
            assert(stmt.fn.parameterList().size() == 1);
            auto& jstr = stmt.fn.parameterList().at(0).value().getString();
            impl_.insert(jstr);
        }
        inline void operator()(const noq::Module::RuleStatement& stmt) {
            impl_.ruleset_[""].ruleList_.push_back(stmt.rule);
        }
        inline void operator()(const noq::Module::VerifyStatement& stmt) {
            impl_.verify(stmt.verify);
        }
    };

    inline void exec(std::istream& ss) {
        noq::Module module;
        module.loadStream(ss);
        StatementExec se(*this);
        for(auto& stmt : module.statementList()){
            std::visit(se, stmt);
        }
    }

    inline Impl(noq::Connection& conn) : conn_(conn) {}
};

void noq::Connection::registerAdaptor(const std::string& name, std::unique_ptr<noq::Factory>&& factory) {
    auto ait = s_factoryList.find(name);
    if(ait != s_factoryList.end()){
        noq::Logger().warn("registering duplicate adaptor:[{0}]", name);
    }
    s_factoryList[name] = std::move(factory);
}

noq::Connection::Connection() {
    impl_ = std::make_unique<Impl>(*this);
    testCB_ = [](const bool&, const std::string&, const std::vector<Function>&, const std::vector<Function>&){};
}

noq::Connection::Connection(const std::string& str) {
    impl_ = std::make_unique<Impl>(*this);
    testCB_ = [](const bool&, const std::string&, const std::vector<Function>&, const std::vector<Function>&){};
    impl_->open(str);
}

noq::Connection::~Connection() {
    impl_->close();
    impl_.reset();
}

bool noq::Connection::hasRuleList(const std::string& ns) const {
    return impl_->hasRuleList(ns);
}

const std::vector<noq::Rule>& noq::Connection::ruleList(const std::string& ns) const {
    return impl_->ruleList(ns);
}

void noq::Connection::open(const std::string& str) {
    return impl_->open(str);
}

void noq::Connection::close() {
    return impl_->close();
}

void noq::Connection::insert(const std::string& obj) const {
    return impl_->insert(obj);
}

void noq::Connection::exec(std::istream& iss) const {
    return impl_->exec(iss);
}

void noq::Connection::execString(const std::string& obj) const {
    std::istringstream ss(obj);
    return exec(ss);
}

void noq::Connection::execFile(const std::string& filename) const {
    std::ifstream ss(filename);
    return exec(ss);
}

std::vector<noq::Value> noq::Connection::queryValueByKey(const noq::Value& key) const {
    return impl_->queryValueByKey(key);
}

std::vector<noq::Value> noq::Connection::queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const {
    return impl_->queryValueByObjectKey(obj, key);
}

std::vector<noq::Value> noq::Connection::queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const {
    return impl_->queryObjectByKeyValue(key, val);
}

void noq::init() {
    noq::Logger::initFile("noql.log");
    noq::Connection::registerAdaptor("json", std::make_unique<noq::JsonAdaptor::JsonFactory>());
#ifdef USE_MONGOC
    noq::Connection::registerAdaptor("mongodb", std::make_unique<noq::MongoAdaptor::MongoFactory>());
#endif
}
