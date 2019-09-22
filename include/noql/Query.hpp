#pragma once
#include "Connection.hpp"

namespace noq {
    struct Query {
        struct iterator {
            inline iterator(std::vector<Function>::const_iterator it) : it_(it) {}
            inline bool operator!=(const iterator& rhs) const {
                return it_ != rhs.it_;
            }
            inline iterator& operator++() {
                ++it_;
                return *this;
            }
            inline const Function& operator*() const {
                return *it_;
            }
        private:
            std::vector<Function>::const_iterator it_;
        };

        inline Query() {}
        inline Query(const std::string& str) {
            compile(str);
        }

        void compile(const std::string& str);
        void exec(const noq::Connection& conn);
        void exec(const std::string& str, const noq::Connection& conn);
        void exec(const noq::Function& qry, const noq::Connection& conn);

        inline void set(const std::string& key, const noq::Value& val) {
            auto nit = variableMap_.find(key);
            if(nit == variableMap_.end()){
                throw noq::Exception("unknown variable:" + key);
            }
            auto idx = nit->second;
            parameterList_[idx] = Parameter(Parameter::Type::Value, ReferenceType::In, val);
        }

        inline auto& getIndex(const std::string& key) const {
            auto nit = variableMap_.find(key);
            if(nit == variableMap_.end()){
                throw noq::Exception("unknown variable:" + key);
            }
            auto& idx = nit->second;
            return idx;
        }

        inline auto& get(const std::string& key, const Function& fn) const {
            auto& idx = getIndex(key);
            return fn.parameterList().at(idx);
        }

        inline iterator begin() {
            return iterator(resl_.begin());
        }
        inline iterator end() {
            return iterator(resl_.end());
        }
        inline auto& resl() const {
            return resl_;
        }

    private:
        std::unique_ptr<Function> parseStringQuery(const std::string& str);
        inline void setQuery(const Function& query);

    private:
        std::unique_ptr<noq::Function> query_;
        std::map<std::string, size_t> variableMap_;
        std::vector<Parameter> parameterList_;

        std::vector<Function> resl_;
    };
}
