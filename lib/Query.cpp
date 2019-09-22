#include "pch.hpp"
#include "noql/Query.hpp"
#include "ExprEval.hpp"
#include "Logger.hpp"

namespace {
    struct EnvBlock {
        std::map<std::string, ::noq::Value> vlist;

        inline void str(std::ostream& os) const {
            os << "{";
            std::string sep;
            for(auto& v : vlist) {
                os << sep << v.first << ":" << v.second;
                sep = ",";
            }
            os << "}";
        }

        inline friend std::ostream& operator<<(std::ostream& os, const EnvBlock& c) {
            c.str(os);
            return os;
        }
    };

    struct ExprEngine {
        const noq::Value& exprList_;
        const EnvBlock& env_;
        std::vector<noq::Value> stack_;
        inline ExprEngine(const noq::Value& exprList, const EnvBlock& env) : exprList_(exprList), env_(env) {}


        inline noq::Value doOp(const std::string& op, const noq::Value& lhs, const noq::Value& rhs) {
            if(op == "+") return (lhs + rhs);
            if(op == "-") return (lhs - rhs);
            if(op == "*") return (lhs * rhs);
            if(op == "/") return (lhs / rhs);
            if(op == "%") return (lhs % rhs);
            if(op == "<") return (lhs < rhs);
            if(op == ">") return (lhs > rhs);
            if(op == "<=") return (lhs <= rhs);
            if(op == ">=") return (lhs >= rhs);
            if(op == "==") return (lhs == rhs);
            if(op == "!=") return (lhs != rhs);
            throw noq::Exception("unknown operator:" + op);
        }
 
        inline noq::Value process() {
            for(auto& exprx : exprList_.getArray()){
                noq::Logger().trace("exprx:{0}", exprx);
                auto& expr = exprx.getObject();
                auto xit = expr.find("type");
                auto xite = expr.end();

                if(xit == xite) {
                    continue;
                }
                auto type = (noq::ExprType)xit->second.getInteger();
                switch(type){
                case noq::ExprType::Operator1:
                    assert(false);
                    break;
                case noq::ExprType::Operator2:
                    if((xit = expr.find("op")) != xite){
                        auto& op = xit->second.getString();
                        auto rhs = stack_.back();
                        stack_.pop_back();
                        auto lhs = stack_.back();
                        stack_.pop_back();
                        auto val = doOp(op, lhs, rhs);
                        stack_.push_back(val);
                    }
                    break;
                case noq::ExprType::VariableRef:
                    if((xit = expr.find("ref")) != xite){
                        auto& vref = xit->second.getString();
                        auto vit = env_.vlist.find(vref);
                        if(vit == env_.vlist.end()){
                            stack_.push_back(noq::Value());
                            break;
                        }
                        auto val = vit->second;
                        stack_.push_back(val);
                    }
                    break;
                case noq::ExprType::Value:
                    if((xit = expr.find("val")) != xite){
                        auto& val = xit->second;
                        stack_.push_back(val);
                    }
                    break;
                default:
                    break;
                }
                for(auto& s : stack_){
                    noq::Logger().trace("-si:{0}", s);
                }
            }
            noq::Value val = stack_.back();
            return val;
        }
    };

    struct QueryEngine {
        const noq::Query& query_;
        const noq::Connection& conn_;

        struct HeadStackItem {
            const noq::Rule* rule;
            inline void addResult(const noq::Function& res) {
                for(auto& r : resl_){
                    if(r.compare(res) == 0){
                        return;
                    }
                }
                resl_.push_back(res);
            }

            inline auto& resl() const {
                return resl_;
            }
            inline HeadStackItem(std::vector<noq::Function>& s) : rule(nullptr), resl_(s) {}
        private:
            std::vector<noq::Function>& resl_;
        };

        std::vector<std::unique_ptr<HeadStackItem>> headStack_;

        struct StackGuard {
            inline StackGuard(std::vector<std::unique_ptr<HeadStackItem>>& stack, std::vector<noq::Function>& resl) : stack_(stack) {
                stack_.push_back(std::make_unique<HeadStackItem>(resl));
            }

            inline ~StackGuard() {
                stack_.pop_back();
            }

            inline auto& resl() {
                return stack_.back()->resl();
            }
        private:
            std::vector<std::unique_ptr<HeadStackItem>>& stack_;
        };

        struct RuleGuard {
            inline RuleGuard(std::vector<std::unique_ptr<HeadStackItem>>& stack, const noq::Rule& rule) : stack_(stack) {
                assert(stack_.size() > 0);
                assert(stack_.back()->rule == nullptr);
                stack_.back()->rule = &rule;
            }

            inline ~RuleGuard(){
                assert(stack_.size() > 0);
                stack_.back()->rule = nullptr;
            }

        private:
            std::vector<std::unique_ptr<HeadStackItem>>& stack_;
        };

        inline bool isRecursive(const noq::Rule& rule) const {
            for(auto& si : headStack_) {
                if(si->rule == &rule){
                    return true;
                }
            }
            return false;
        }

        inline auto convertValToArray(const noq::Value& val) const {
            std::vector<noq::Value> valList;
            if(val.isArray()){
                auto& arr = val.getArray();
                for(auto& v : arr){
                    valList.push_back(v);
                }
            }else{
                valList.push_back(val);
            }
            return valList;
        }

        inline EnvBlock scatter(const noq::Function& from, const noq::Function& to, const EnvBlock& senv) const {
            assert(from.arity() == to.arity());
            EnvBlock env = senv;
            auto fit = from.parameterList().begin();
            auto fite = from.parameterList().end();
            auto tit = to.parameterList().begin();
            auto tite = to.parameterList().end();
            while((fit != fite) && (tit != tite)){
                if((fit->type() == noq::Parameter::Type::Value) && (tit->type() == noq::Parameter::Type::Variable)){
                    env.vlist.emplace(tit->value().getString(), fit->value());
                }
                ++fit;
                ++tit;
            }
            noq::Logger().trace("{0}:scatter:env:{1}", headStack_.size(), env);
            return env;
        }

        inline noq::Function gather(const noq::Function& from, const EnvBlock& env, const noq::Function::Type& type) const {
            auto fit = from.parameterList().begin();
            auto fite = from.parameterList().end();
            std::vector<noq::Parameter> parameterList;
            noq::Logger().trace("{0}:gather:env:{1}", headStack_.size(), env);
            while(fit != fite){
                decltype(EnvBlock::vlist)::const_iterator eit;
                if((fit->type() == noq::Parameter::Type::Variable) && ((eit = env.vlist.find(fit->value().getString())) != env.vlist.end())){
                    parameterList.push_back(noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, eit->second));
                }else{
                    parameterList.push_back(*fit);
                }
                ++fit;
            }

            noq::Function to(type, from.ns(), from.name(), parameterList);
            noq::Logger().trace("{0}:gather:to:{1}", headStack_.size(), to);
            return to;
        }

        struct CacheItem {
            std::vector<noq::Function> flist;
        };
        std::map<noq::Function, std::unique_ptr<CacheItem>> cache_;

        inline const auto& processQuery(const noq::Function& qry) {
            auto cit = cache_.find(qry);
            if(cit == cache_.end()){
                cache_[qry] = std::make_unique<CacheItem>();
                cit = cache_.find(qry);
                StackGuard sg(headStack_, cit->second->flist);
                processHead(qry);
            }
            assert(cit != cache_.end());
            return cit->second->flist;
        }

        void processBody(const std::vector<noq::Function>::const_iterator& it, const EnvBlock& env) {
            auto& part = *it;
            noq::Logger().trace("{0}:processBody:part[{1}], env:[{2}]", headStack_.size(), part, env);
            if(part.type() == noq::Function::Type::EndRule){
                auto res = gather(part, env, noq::Function::Type::Regular);
                noq::Logger().trace("{0}:endBody[{1}]:[{2}]", headStack_.size(), res, env);
                headStack_.back()->addResult(res);
                return;
            }

            if((part.type() == noq::Function::Type::Assign) || (part.type() == noq::Function::Type::Logical)) {
                noq::Logger().trace("{0}:ExprEval:qry:[{1}]", headStack_.size(), part.parameterList().at(0).value());
                ExprEngine ee(part.parameterList().at(0).value(), env);
                auto res = ee.process();
                noq::Logger().trace("{0}:ExprEval:qry:[{1}], res:[{2}]", headStack_.size(), part.parameterList().at(0).value(), res);
                if(res.isNull()){
                    return;
                }
                auto nenv = env;
                if(part.type() == noq::Function::Type::Assign){
                    assert(part.parameterList().size() == 2);
                    auto& var = part.parameterList().at(1);
                    nenv.vlist[var.value().getString()] = res;
                }else{
                    if((!res.isBoolean()) || (!res.getBoolean())){
                        return;
                    }
                }
                processBody((it+1), nenv);
                return;
            }

            auto nqry = gather(part, env, part.type());
            noq::Logger().trace("{0}:processBody:part[{1}], env:[{2}], nqry:[{3}]", headStack_.size(), part, env, nqry);
            auto& nresl = processQuery(nqry);

            for(auto& res : nresl){
                EnvBlock nenv = scatter(res, part, env);
                noq::Logger().trace("{0}:processBody:res[{1}], part:[{2}], nenv:[{3}]", headStack_.size(), res, part, nenv);
                processBody((it+1), nenv);
            }
        }

        void processHead(const noq::Function& qry) {
            noq::Logger().info("{0}:processHead:part[{1}]", headStack_.size(), qry);

            if(conn_.hasRuleList(qry.ns())){
                auto& ruleList = conn_.ruleList(qry.ns());
                for(auto& rule : ruleList){
                    noq::Logger().trace("{0}:processHead:check:rule:[{1}]", headStack_.size(), rule);
                    assert(rule.head().type() == noq::Function::Type::BeginRule);
                    if(rule.head().name() != qry.name()){
                        noq::Logger().trace("{0}:processHead:skip(name mismatch):[{1}]", headStack_.size(), rule);
                        continue;
                    }

                    if(rule.head().arity() != qry.arity()){
                        noq::Logger().trace("{0}:processHead:skip(arity mismatch):[{1}]", headStack_.size(), rule);
                        continue;
                    }

                    if(isRecursive(rule)){
                        noq::Logger().trace("{0}:processHead:skip(recursive):[{1}]", headStack_.size(), rule);
                        continue;
                    }

                    noq::Logger().info("{0}:processHead:execute:[{1}]", headStack_.size(), rule);
                    EnvBlock nenv = scatter(qry, rule.head(), EnvBlock());
                    RuleGuard rg(headStack_, rule);
                    processBody(rule.body().begin(), nenv);
                }
            }

            if(qry.arity() == 1) {
                noq::Logger().trace("{0}:processHead:native-call-1:[{1}]", headStack_.size(), qry);
                auto& key = qry.name();
                auto& p0 = qry.parameterList().at(0);

                // KeyVal == ValVal or KeyVal == ValVar
                auto valList = conn_.queryValueByKey(key);
                noq::Logger().trace("{0}:processHead:native-call-1:[{1}], valList:[{2}] (ValValVal or ValValVar)", headStack_.size(), qry, valList.size());
                for(auto& vali : valList){
                    auto avalList = convertValToArray(vali);
                    for(auto& val : avalList){
                        std::vector<noq::Parameter> parameterList;
                        parameterList.push_back(noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, val));
                        auto fn = noq::Function(qry.type(), qry.ns(), qry.name(), parameterList);
                        noq::Logger().trace("{0}:processHead:native-call-1:qry:[{1}], res:[{2}], fn:[{3}] *********", headStack_.size(), qry, val, fn);
                        headStack_.back()->addResult(fn);
                    }
                }
                noq::Logger().trace("{0}:processHead:native-call-1-done:qry:[{1}], resl:[{2}]", headStack_.size(), qry, headStack_.back()->resl().size());
            }else if(qry.arity() == 2) {
                noq::Logger().trace("{0}:processHead:native-call:[{1}]", headStack_.size(), qry);
                auto& key = qry.name();
                auto& p0 = qry.parameterList().at(0);
                auto& p1 = qry.parameterList().at(1);

                if(p0.type() == noq::Parameter::Type::Value) {
                    // ObjKeyVal == ValValVal or ObjKeyVal == ValValVar
                    auto valList = conn_.queryValueByObjectKey(p0.value(), key);
                    noq::Logger().trace("{0}:processHead:native-call:[{1}], valList:[{2}] (ValValVal or ValValVar)", headStack_.size(), qry, valList.size());
                    for(auto& vali : valList){
                        auto avalList = convertValToArray(vali);
                        for(auto& val : avalList){
                            std::vector<noq::Parameter> parameterList;
                            parameterList.push_back(p0);
                            parameterList.push_back(noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, val));
                            auto fn = noq::Function(qry.type(), qry.ns(), qry.name(), parameterList);
                            noq::Logger().trace("{0}:processHead:native-call1:qry:[{1}], res:[{2}], fn:[{3}] *********", headStack_.size(), qry, val, fn);
                            headStack_.back()->addResult(fn);
                        }
                    }
                }else if(p1.type() == noq::Parameter::Type::Value) {
                    // ObjKeyVal == VarValVal
                    auto valList = conn_.queryObjectByKeyValue(key, p1.value());
                    noq::Logger().trace("{0}:processHead:native-call:[{1}], valList:[{2}] (VarValVal)", headStack_.size(), qry, valList.size());
                    for(auto& vali : valList){
                        auto avalList = convertValToArray(vali);
                        for(auto& val : avalList){
                            std::vector<noq::Parameter> parameterList;
                            parameterList.push_back(noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, val));
                            parameterList.push_back(p1);
                            auto res = noq::Function(qry.type(), qry.ns(), qry.name(), parameterList);
                            noq::Logger().trace("{0}:processHead:native-call2:qry:[{1}], res:[{2}], fn:[{3}] ***********", headStack_.size(), qry, val, res);
                            headStack_.back()->addResult(res);
                        }
                    }
                }
                noq::Logger().trace("{0}:processHead:native-call-done:qry:[{1}], resl:[{2}]", headStack_.size(), qry, headStack_.back()->resl().size());
            }
        }

        void process(const noq::Function& qry, std::vector<noq::Function>& resl) {
            QueryEngine::StackGuard sg(headStack_, resl);
            return processHead(qry);
        }

        inline QueryEngine(const noq::Query& query, const noq::Connection& conn) : query_(query), conn_(conn) {}
    };
}

inline void noq::Query::setQuery(const noq::Function& query) {
    query_ = std::make_unique<Function>(query);
    parameterList_ = query_->parameterList();
    for(size_t i = 0; i < parameterList_.size(); ++i){
        auto& parameter = parameterList_.at(i);
        if(parameter.type() == noq::Parameter::Type::Variable){
            variableMap_[parameter.value().getString()] = i;
        }
    }
}

void noq::Query::compile(const std::string& str) {
    Module module;
    module.loadString(str);
    auto& qry = module.getQuery();
    setQuery(qry);
}

void noq::Query::exec(const noq::Connection& conn) {
    assert(query_);
    resl_.clear();
    auto nquery = Function(query_->type(), query_->ns(), query_->name(), parameterList_);
    QueryEngine qe(*this, conn);
    qe.process(nquery, resl_);
}

void noq::Query::exec(const std::string& str, const noq::Connection& conn) {
    compile(str);
    exec(conn);
}

void noq::Query::exec(const noq::Function& qry, const noq::Connection& conn) {
    setQuery(qry);
    exec(conn);
}

