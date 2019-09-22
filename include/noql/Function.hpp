#pragma once

#include "Value.hpp"

namespace noq {
    enum class ReferenceType {
        In,
        Out,
        Any
    };

    struct Parameter {
        enum class Type {
            Value,
            Variable,
            NoVariable
        };

        inline Parameter(const Type& pt, const ReferenceType& rt, const ::noq::Value& value)
        : type_(pt), referenceType_(rt), value_(value){}

        inline auto& type() const {
            return type_;
        }

        inline auto& direction() const {
            return referenceType_;
        }

        inline auto& value() const {
            return value_;
        }

        int compare(const Parameter& rhs) const;

        inline void str(std::ostream& os) const {
            switch(type_){
            case Type::Value:
                os << value_;
                break;
            case Type::NoVariable:
                os << "@";
                break;
            case Type::Variable:
                if(referenceType_ == ReferenceType::In){
                    os << "&" << value_.getString();
                }else{
                    os << "@" << value_.getString();
                }
                break;
            }
        }

        inline friend std::ostream& operator<<(std::ostream& os, const Parameter& c) {
            c.str(os);
            return os;
        }
    private:
        Type type_;
        ReferenceType referenceType_;
        noq::Value value_;
    };

    struct Function {
        enum class Type {
            Regular,
            Logical,
            Assign,
            BeginRule,
            EndRule
        };

        inline Function(const Type& type, const std::string& ns, const noq::Value& name, const std::vector<Parameter>& parameterList) 
        : type_(type), ns_(ns), name_(name), parameterList_(parameterList) {}

        inline auto& type() const {
            return type_;
        }

        inline auto& ns() const {
            return ns_;
        }

        inline auto& name() const {
            return name_;
        }

        inline auto arity() const {
            return parameterList_.size();
        }

        inline auto& parameterList() const {
            return parameterList_;
        }

        int compare(const Function& rhs) const;

        inline bool operator<(const Function& rhs) const {
            return (compare(rhs) < 0);
        }

        inline void str(std::ostream& os) const {
            os << name_ << "(";
            std::string sep;
            for(auto& p : parameterList_){
                os << sep;
                p.str(os);
                sep = ",";
            }
            os << ")";
        }

        inline friend std::ostream& operator<<(std::ostream& os, const Function& c) {
            c.str(os);
            return os;
        }
    private:
        const Type type_;
        const std::string ns_;
        const noq::Value name_;
        const std::vector<Parameter> parameterList_;
    };
}