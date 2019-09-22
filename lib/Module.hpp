#pragma once

#include "noql/Value.hpp"
#include "noql/Function.hpp"
#include "noql/Url.hpp"

namespace noq {
    // do not change this order or values
    enum class ExprType {
        Value = 1,
        VariableRef = 2,
        Operator1 = 3,
        Operator2 = 4,
    };

    struct Rule {
        inline Rule(const Function& head, const std::vector<Function>& body) : head_(head), body_(body) {}
        inline auto& head() const {
            return head_;
        }
        inline auto& body() const {
            return body_;
        }

        inline void str(std::ostream& os) const {
            os << head_ << " := ";
            std::string sep;
            for(auto& f : body_){
                os << sep << f;
                sep = " && ";
            }
        }

        inline friend std::ostream& operator<<(std::ostream& os, const Rule& c) {
            c.str(os);
            return os;
        }

    private:
        const Function head_;
        const std::vector<Function> body_;
    };

    class Verify {
    public:
        Function query_;
        std::vector<Function> result_;
        inline Verify(Function query) : query_(query){}
    };

    class Connection;
    class Module {
    public:
        struct RuleStatement {
            Rule rule;
            inline RuleStatement(const Rule& r) : rule(r) {}
        };
        struct InsertDocumentStatement {
            Function fn;
            inline InsertDocumentStatement(const Function& f) : fn(f) {}
        };
        struct InsertPairStatement {
            Function fn;
            inline InsertPairStatement(const Function& f) : fn(f) {}
        };
        struct VerifyStatement {
            Verify verify;
            inline VerifyStatement(const Verify& v) : verify(v) {}
        };

        typedef std::variant<
            RuleStatement,
            InsertDocumentStatement,
            InsertPairStatement,
            VerifyStatement
        > Statement;

    public:
        inline Module(){}
        inline Module(std::istream& is) {
            loadStreamModule(is);
        }

        inline void loadString(const std::string& str) {
            std::istringstream iss(str);
            loadStreamModule(iss);
        }

        inline void loadFile(const std::string& fname) {
            std::ifstream iss(fname);
            loadStreamModule(iss);
        }

        inline void loadStream(std::istream& iss) {
            loadStreamModule(iss);
        }

        inline void str(std::ostream& os) const {
        }

        inline friend std::ostream& operator<<(std::ostream& os, const Module& c) {
            c.str(os);
            return os;
        }

        inline auto& statementList() const {
            return statementList_;
        }

        inline bool hasQuery() const {
            return (bool)query_;
        }

        inline auto& getQuery() const {
            if(!query_){
                throw noq::Exception("input must contain exactly one query");
            }
            return *query_;
        }

        inline void addInsertDocumentStatement(const Function& fn) {
            statementList_.push_back(InsertDocumentStatement(fn));
        }
        inline void addInsertPairStatement(const Function& fn) {
            statementList_.push_back(InsertPairStatement(fn));
        }
        inline void addRuleStatement(const Rule& rule) {
            statementList_.push_back(RuleStatement(rule));
        }
        inline void addVerifyStatement(const Verify& verify) {
            statementList_.push_back(VerifyStatement(verify));
        }
        inline void addQueryStatement(const Function& query) {
            query_ = std::make_unique<Function>(query);
        }
    private:
        void loadStreamModule(std::istream& is);

    private:
        std::vector<Statement> statementList_;
        std::unique_ptr<Function> query_;
    };
}
