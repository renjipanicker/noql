#pragma once

namespace noq {
    struct Operator2 {
        struct Add{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<decltype(std::declval<Lhs>() + std::declval<Rhs>()), decltype(std::declval<Lhs>() + std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs + rhs);
            }
        };

        struct Subtract{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<decltype(std::declval<Lhs>() - std::declval<Rhs>()), decltype(std::declval<Lhs>() - std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs - rhs);
            }
        };

        struct Multiply{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<decltype(std::declval<Lhs>() * std::declval<Rhs>()), decltype(std::declval<Lhs>() * std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs * rhs);
            }
        };

        struct Divide{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<decltype(std::declval<Lhs>() / std::declval<Rhs>()), decltype(std::declval<Lhs>() / std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs / rhs);
            }
        };

        struct Modulo{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<decltype(std::declval<Lhs>() % std::declval<Rhs>()), decltype(std::declval<Lhs>() % std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs % rhs);
            }
        };

        struct LT{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() < std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs < rhs);
            }
        };

        struct LTE{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() <= std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs <= rhs);
            }
        };

        struct GT{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() > std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs > rhs);
            }
        };

        struct GTE{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() >= std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs >= rhs);
            }
        };

        struct EQ{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() == std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs == rhs);
            }
        };

        struct NEQ{
            template <typename LhsT, typename RhsT>
            struct has{
                template <typename Lhs, typename Rhs>
                static constexpr auto test(Lhs* lhs, Rhs* rhs) ->
                typename std::is_same<bool, decltype(std::declval<Lhs>() != std::declval<Rhs>())>::type
                { return {}; }

                template <typename Lhs, typename Rhs>
                static constexpr auto test(...) ->
                std::false_type
                { return {};}

                static constexpr bool value = test<LhsT, RhsT>(nullptr, nullptr);
            };

            template <typename Lhs, typename Rhs>
            static inline auto exec(const Lhs& lhs, const Rhs& rhs) {
                return (lhs != rhs);
            }
        };
    };

    template<typename VariantT, typename NullT>
    struct VariantEval {

        template<typename LhsT, typename RhsT, typename OpT>
        using EnableIfOp = typename std::enable_if<OpT::template has<LhsT,RhsT>::value, int>::type;

        template<typename LhsT, typename RhsT, typename OpT>
        using DisableIfOp = typename std::enable_if<!OpT::template has<LhsT,RhsT>::value, int>::type;

        template <typename LhsT, typename RhsT, typename OpT, EnableIfOp<LhsT, RhsT, OpT>...>
        static inline auto
        invoke(const LhsT& lhs, const RhsT& rhs, const OpT& op) {
            return OpT::exec(lhs, rhs);
        }

        template <typename LhsT, typename RhsT, typename OpT, DisableIfOp<LhsT, RhsT, OpT>...>
        static inline auto
        invoke(const LhsT& lhs, const RhsT& rhs, const OpT& op){
            return NullT();
        }

        template <typename OpT>
        static inline auto do_op(const VariantT& lhs, const VariantT& rhs) {
            VariantT ret;
            std::visit([&ret, &rhs](const auto& lval){
                std::visit([&ret, &lval, &rhs](const auto& rval){
                    auto retx = invoke(lval, rval, OpT());
                    ret = VariantT(retx);
                }, rhs);
            }, lhs);

            return ret;
        }
    };

    class Value;
    struct Null{};
    struct Float {
        double d;
        inline Float(const double& v) : d(v){}
        inline Float(const float& v) : d(v){}
    };
    typedef std::vector<Value> Array;
    typedef std::map<std::string,Value> Object;

    inline std::ostream& operator<<(std::ostream& os, const noq::Null&) {
        os << "<null>";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const noq::Float& val) {
        os << val.d;
        return os;
    }

    class Value {
        enum class Index {
            NUll = 0,
            Integer = 1,
            Unsigned = 2,
            Float = 3,
            Bool = 4,
            String = 5,
            Array = 6,
            Object = 7,
        };

        typedef std::variant<
            Null,         // 0
            int,          // 1
            unsigned int, // 2
            Float,        // 3
            bool,         // 4
            std::string,  // 5
            Array,        // 6
            Object        // 7
        > Value_;

        struct Visitor {
            std::ostream& os_;
            inline Visitor(std::ostream& os) : os_(os) {}

            template <typename T>
            inline void operator()(const T& val) {
                os_ << val;
            }

            inline void operator()(const std::string& val) {
                os_ << "\"" << val << "\"";
            }

            inline void operator()(const Array& val) {
                os_ << "[";
                std::string sep;
                for(auto& i : val){
                    os_ << sep << i;
                    sep = ",";
                }
                os_ << "]";
            }

            inline void operator()(const Object& val) {
                os_ << "{";
                std::string sep;
                for(auto& i : val){
                    os_ << sep << i.first << ":" << i.second;
                    sep = ",";
                }
                os_ << "}";
            }
        };

    public:
        inline Value() : val_(Null()) {
        }

        template<typename T>
        inline Value(const T& v) : val_(v) {
        }

        inline Value(const char* v) : val_(std::string(v)) {
        }

        inline Value(char* v) : val_(std::string(v)) {
        }

        inline Value(const float& v) : val_(Float(v)) {
        }

        inline Value(const double& v) : val_(Float(v)) {
        }

        inline auto& val() const {
            return val_;
        }

        inline auto operator+(const Value& rhs) const {
            return VariantEval<Value_, Null>::do_op<Operator2::Add>(val_, rhs.val_);
        }
        inline auto operator-(const Value& rhs) const {
            return VariantEval<Value_, Null>::do_op<Operator2::Subtract>(val_, rhs.val_);
        }
        inline auto operator*(const Value& rhs) const {
            return VariantEval<Value_, Null>::do_op<Operator2::Multiply>(val_, rhs.val_);
        }
        inline auto operator/(const Value& rhs) const {
            return VariantEval<Value_, Null>::do_op<Operator2::Divide>(val_, rhs.val_);
        }
        inline auto operator%(const Value& rhs) const {
            return VariantEval<Value_, Null>::do_op<Operator2::Modulo>(val_, rhs.val_);
        }

        inline bool operator<(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::LT>(val_, rhs.val_);
            return std::get<bool>(ret);
        }
        inline bool operator<=(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::LTE>(val_, rhs.val_);
            return std::get<bool>(ret);
        }
        inline bool operator>(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::GT>(val_, rhs.val_);
            return std::get<bool>(ret);
        }
        inline bool operator>=(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::GTE>(val_, rhs.val_);
            return std::get<bool>(ret);
        }
        inline bool operator==(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::EQ>(val_, rhs.val_);
            return std::get<bool>(ret);
        }
        inline bool operator!=(const Value& rhs) const {
            auto ret = VariantEval<Value_, Null>::do_op<Operator2::NEQ>(val_, rhs.val_);
            return std::get<bool>(ret);
        }

    private:
        template<typename T>
        inline auto& get() const {
            return std::get<T>(val_);
        }
    public:
        inline auto isNull() const {
            return val_.index() == (size_t)Index::NUll;
        }

        inline auto isFloat() const {
            return val_.index() == (size_t)Index::Float;
        }

        inline auto isInteger() const {
            return val_.index() == (size_t)Index::Integer;
        }

        inline auto isUnsigned() const {
            return val_.index() == (size_t)Index::Unsigned;
        }

        inline auto isString() const {
            return val_.index() == (size_t)Index::String;
        }

        inline auto isBoolean() const {
            return val_.index() == (size_t)Index::Bool;
        }

        inline auto isArray() const {
            return val_.index() == (size_t)Index::Array;
        }

        inline auto isObject() const {
            return val_.index() == (size_t)Index::Object;
        }

        inline auto& getInteger() const {
            assert(isInteger());
            return get<int>();
        }

        inline auto& getUnsigned() const {
            assert(isUnsigned());
            return get<unsigned int>();
        }

        inline auto& getString() const {
            assert(isString());
            return get<std::string>();
        }

        inline auto& getBoolean() const {
            assert(isBoolean());
            return get<bool>();
        }

        inline auto& getFloat() const {
            assert(isFloat());
            return get<Float>().d;
        }

        inline auto& getArray() const {
            return get<Array>();
        }

        inline auto& getObject() const {
            return get<Object>();
        }

        inline void str(std::ostream& os) const {
            std::visit(Visitor(os), val_);
        }

        inline friend std::ostream& operator<<(std::ostream& os, const Value& c) {
            c.str(os);
            return os;
        }
    private:
        Value_ val_;
    };

}

