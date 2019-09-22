%include {
    #include "pch.hpp"
    #include "noql/Query.hpp"
    #include "Module.hpp"

    namespace {
        struct Context {
            noq::Module* module;
            inline Context(noq::Module* m) : module(m) {}

            std::ostringstream jsonStr;

            std::unique_ptr<noq::Parameter> parameter;
            std::vector<noq::Parameter> parameterList;
            std::unique_ptr<noq::Function> function;
            std::vector<noq::Function> functionList;
            std::unique_ptr<noq::Function> ruleHead;
            bool inRuleHead = false;

            size_t insertArity;

            std::unique_ptr<noq::Verify> verify;

            std::map<std::string, int> variableList;

            noq::Array exprList;

            inline bool hasVariable(const std::string& vname) {
                auto vit = variableList.find(vname);
                if(vit != variableList.end()){
                    return true;
                }
                variableList[vname] = 1;
                return false;
            }
            inline void addVariableRefExpr(const std::string& vname) {
                noq::Object val;
                val["type"] = (int)noq::ExprType::VariableRef;
                val["ref"] = vname;
                exprList.push_back(val);
            }

            inline void addOperator1Expr(const std::string& op) {
                noq::Object val;
                val["type"] = (int)noq::ExprType::Operator1;
                val["op"] = op;
                exprList.push_back(val);
            }

            inline void addOperator2Expr(const std::string& op) {
                noq::Object val;
                val["type"] = (int)noq::ExprType::Operator2;
                val["op"] = op;
                exprList.push_back(val);
            }

            inline void addFloatConstExpr(const std::string& str) {
                float ival = std::atof(str.c_str());
                noq::Object val;
                val["type"] = (int)noq::ExprType::Value;
                val["val"] = ival;
                exprList.push_back(val);
            }

            inline void addIntConstExpr(const std::string& str) {
                int ival = std::atol(str.c_str());
                noq::Object val;
                val["type"] = (int)noq::ExprType::Value;
                val["val"] = ival;
                exprList.push_back(val);
            }

            static inline void addStringConstExpr(const std::string& str, noq::Array& expList) {
                noq::Object val;
                val["type"] = (int)noq::ExprType::Value;
                val["val"] = str;
                expList.push_back(val);
            }

            inline void addStringConstExpr(const std::string& str) {
                addStringConstExpr(str, exprList);
            }

            inline void addBoolConstExpr(const bool& str) {
                noq::Object val;
                val["type"] = (int)noq::ExprType::Value;
                val["val"] = str;
                exprList.push_back(val);
            }
        };
    }

    std::unique_ptr<Context> s_ctx;
}

%syntax_error {
    throw noq::Exception("FileParser: syntax error");
}


%start_symbol rModule

%left LCURLY RCURLY.
%left OR.
%left AND.
%left NOT.
%left ASSIGN.
%left DEEP_EQ SHALLOW_EQ NEQ.
%left LT GT LTE GTE.
%left PLUS MINUS.
%left STAR DIVIDE PERCENT.
%left LBRACKET RBRACKET.

/////////////////////////////////////////////////////////////////////////////////////////////
// module definition
rModule ::= rStmtList.
rModule ::= rQueryStmt.

rStmtList ::= rStmtList rStmtItem.
rStmtList ::= rStmtItem.

rStmtItem ::= rInsertStmt.
rStmtItem ::= rRuleStmt.
rStmtItem ::= rVerifyStmt.

/////////////////////////////////////////////////////////////////////////////////////////////
// QUERY definition
rQueryStmt ::= rFunctionExpr. {
    assert(s_ctx->module != nullptr);
    s_ctx->module->addQueryStatement(*(s_ctx->function));
}

rQueryStmt ::= rFunctionExpr SEMI. {
    assert(s_ctx->module != nullptr);
    s_ctx->module->addQueryStatement(*(s_ctx->function));
}

/////////////////////////////////////////////////////////////////////////////////////////////
// INSERT definition
rInsertStmt ::= rJsonInit rJsonObj SEMI. {
    assert(s_ctx->module != nullptr);
    std::vector<noq::Parameter> parameterList;
    auto parameter = noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, s_ctx->jsonStr.str());
    parameterList.push_back(parameter);

    auto function = noq::Function(noq::Function::Type::Regular, "", "$INSERT", parameterList);
    if(s_ctx->insertArity == 2){
        s_ctx->module->addInsertPairStatement(function);
    }else if(s_ctx->insertArity == 3){
        s_ctx->module->addInsertDocumentStatement(function);
    }
}

rJsonInit ::= INSERT DOCUMENT. {
    s_ctx->jsonStr.str("");
    s_ctx->insertArity = 3;
}

rJsonInit ::= INSERT PAIR. {
    s_ctx->jsonStr.str("");
    s_ctx->insertArity = 2;
}

// Json Array Definition
rJsonArr ::= rJsonArrInit rJsonArrList RSQUARE. {
    s_ctx->jsonStr << "]";
}

rJsonArrInit ::= LSQUARE. {
    s_ctx->jsonStr << "[";
}

rJsonArrList ::= rJsonArrList rJsonArrListSep rJsonItem.
rJsonArrList ::= rJsonItem.

rJsonArrListSep ::= COMMA. {
    s_ctx->jsonStr << ",";
}

// Json Object Definition
rJsonObj ::= rJsonObjInit rJsonObjList COMMA RCURLY. {
    s_ctx->jsonStr << "}";
}

rJsonObj ::= rJsonObjInit rJsonObjList RCURLY. {
    s_ctx->jsonStr << "}";
}

rJsonObjInit ::= LCURLY. {
    s_ctx->jsonStr << "{";
}

rJsonObjList ::= rJsonObjList rJsonObjListSep rJsonObjPair.
rJsonObjList ::= rJsonObjPair.

rJsonObjListSep ::= COMMA. {
    s_ctx->jsonStr << ",";
}

rJsonObjPair ::= rJsonObjKey rJsonObjPairSep rJsonItem.

rJsonObjPairSep ::= COLON. {
    s_ctx->jsonStr << ":";
}

rJsonObjKey ::= STRING(T). {
    s_ctx->jsonStr << '"' << T.buf << '"';
}

rJsonItem ::= rJsonObj.
rJsonItem ::= rJsonArr.

rJsonItem ::= F_NUMBER(T). {
    s_ctx->jsonStr << T.buf;
}

rJsonItem ::= I_NUMBER(T). {
    s_ctx->jsonStr << T.buf;
}

rJsonItem ::= STRING(T). {
    s_ctx->jsonStr << '"' << T.buf << '"';
}

rJsonItem ::= TRUE. {
    s_ctx->jsonStr << "true";
}

rJsonItem ::= FALSE. {
    s_ctx->jsonStr << "false";
}

rJsonItem ::= NULLV. {
    s_ctx->jsonStr << "null";
}

/////////////////////////////////////////////////////////////////////////////////////////////
// verify definition
rVerifyStmt ::= VERIFY rVerifyHead COLON_EQ rVerifyList SEMI. {
    assert(s_ctx->module != nullptr);
    s_ctx->module->addVerifyStatement(*(s_ctx->verify));
}

rVerifyHead ::= rFunctionExpr. {
    s_ctx->verify = std::make_unique<noq::Verify>(*(s_ctx->function));
}

rVerifyList ::= LSQUARE rVerifyListX RSQUARE.
rVerifyList ::= LSQUARE RSQUARE.

rVerifyListX ::= rVerifyListX COMMA rVerifyItem.
rVerifyListX ::= rVerifyItem.

rVerifyItem ::= rFunctionExpr. {
    s_ctx->verify->result_.push_back(*(s_ctx->function));
}

/////////////////////////////////////////////////////////////////////////////////////////////
// rule definition
rRuleStmt ::= rRuleInit rRuleHead COLON_EQ rRuleBody SEMI. {
    assert(s_ctx->module != nullptr);
    std::vector<noq::Parameter> parameterList;
    auto function = noq::Function(noq::Function::Type::EndRule, s_ctx->ruleHead->ns(), s_ctx->ruleHead->name(), s_ctx->ruleHead->parameterList());
    s_ctx->functionList.push_back(function);
    s_ctx->module->addRuleStatement(noq::Rule(*(s_ctx->ruleHead), s_ctx->functionList));
    s_ctx->variableList.clear();
}

rRuleInit ::= RULE. {
    s_ctx->inRuleHead = true;
}

rRuleHead ::= rFunctionExpr. {
    s_ctx->ruleHead = std::move(s_ctx->function);
    s_ctx->inRuleHead = false;
}

rRuleBody ::= rRuleBody AND rRuleItem. {
    s_ctx->functionList.push_back(*(s_ctx->function));
}

rRuleBody ::= rRuleItem. {
    s_ctx->functionList.clear();
    s_ctx->functionList.push_back(*(s_ctx->function));
}

rRuleItem ::= rFunctionExpr.
rRuleItem ::= rFullExpr.

/////////////////////////////////////////////////////////////////////////////////////////////
// expression definition
rFullExpr ::= LBRACKET VARIABLE(V) ASSIGN rLogicalExpr RBRACKET. {
    std::vector<noq::Parameter> parameterList;
    auto parameter = noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, s_ctx->exprList);
    parameterList.push_back(parameter);
    auto parameter2 = noq::Parameter(noq::Parameter::Type::Variable, noq::ReferenceType::Any, noq::Value(std::string(V.buf)));
    parameterList.push_back(parameter2);
    s_ctx->function = std::make_unique<noq::Function>(noq::Function::Type::Assign, "", "$ASSIGN_EXPR", parameterList);
}

rFullExpr ::= LBRACKET rLogicalExpr RBRACKET. {
    std::vector<noq::Parameter> parameterList;
    auto parameter = noq::Parameter(noq::Parameter::Type::Value, noq::ReferenceType::Any, s_ctx->exprList);
    parameterList.push_back(parameter);
    s_ctx->function = std::make_unique<noq::Function>(noq::Function::Type::Logical, "", "$LOGICAL_EXPR", parameterList);
}

rOrderedExpr ::= LBRACKET rMathExpr RBRACKET.
rMathExpr ::= rOrderedExpr.

// logical expressions
rLogicalExpr ::= rLogicalExpr AND(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr OR(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= NOT(O) rLogicalExpr. {
    s_ctx->addOperator1Expr(O.buf);
}

// comparison expressions
rLogicalExpr ::= rLogicalExpr LT(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr LTE(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr GT(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr GTE(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr DEEP_EQ(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr SHALLOW_EQ(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rLogicalExpr NEQ(O) rLogicalExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rLogicalExpr ::= rMathExpr.

// comparison expressions
rMathExpr ::= rMathExpr PLUS(O) rMathExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rMathExpr ::= rMathExpr MINUS(O) rMathExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rMathExpr ::= rMathExpr STAR(O) rMathExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rMathExpr ::= rMathExpr DIVIDE(O) rMathExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

rMathExpr ::= rMathExpr PERCENT(O) rMathExpr. {
    s_ctx->addOperator2Expr(O.buf);
}

// prrimary expressions
rMathExpr ::= F_NUMBER(V). {
    s_ctx->addFloatConstExpr(V.buf);
}

rMathExpr ::= I_NUMBER(V). {
    s_ctx->addIntConstExpr(V.buf);
}

rMathExpr ::= STRING(V). {
    s_ctx->addStringConstExpr(V.buf);
}

rMathExpr ::= TRUE. {
    s_ctx->addBoolConstExpr(true);
}

rMathExpr ::= FALSE. {
    s_ctx->addBoolConstExpr(false);
}

rMathExpr ::= VARIABLE(V). {
    s_ctx->addVariableRefExpr(V.buf);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// function definition
rFunctionExpr ::= rFunctionName(I) rParameterList. {
    auto ftype = noq::Function::Type::Regular;
    if(s_ctx->inRuleHead){
        ftype = noq::Function::Type::BeginRule;
    }
    s_ctx->function = std::make_unique<noq::Function>(ftype, "", I.buf, s_ctx->parameterList);
}

rFunctionExpr ::= IDENTIFIER(N) COLON rFunctionName(I) rParameterList. {
    auto ftype = noq::Function::Type::Regular;
    if(s_ctx->inRuleHead){
        ftype = noq::Function::Type::BeginRule;
    }
    s_ctx->function = std::make_unique<noq::Function>(ftype, N.buf, I.buf, s_ctx->parameterList);
}

rFunctionName(L) ::= IDENTIFIER(R). {L = R;}
rFunctionName(L) ::= STRING(R). {L = R;}

rParameterList ::= rParameterListX RBRACKET.

rParameterList ::= LBRACKET RBRACKET. {
    s_ctx->parameterList.clear();
}

rParameterListX ::= rParameterListX COMMA rParameter. {
    s_ctx->parameterList.push_back(*(s_ctx->parameter));
}

rParameterListX ::= LBRACKET rParameter. {
    s_ctx->parameterList.clear();
    s_ctx->parameterList.push_back(*(s_ctx->parameter));
}

rParameter ::= AVARIABLE. {
    noq::Value val;
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::NoVariable, noq::ReferenceType::Any, val);
}

rParameter ::= VARIABLE(V). {
    auto dir = noq::ReferenceType::Any;
    if(s_ctx->hasVariable(V.buf)){
        dir = noq::ReferenceType::In;
    }else{
        dir = noq::ReferenceType::Out;
    }
    noq::Value val(std::string(V.buf));
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Variable, dir, val);
}

rParameter ::= STRING(V). {
    noq::Value val(std::string(V.buf));
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Value, noq::ReferenceType::Any, val);
}

rParameter ::= F_NUMBER(V). {
    double d = std::atof(V.buf);
    noq::Value val(d);
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Value, noq::ReferenceType::Any, val);
}

rParameter ::= I_NUMBER(V). {
    int d = std::atoi(V.buf);
    noq::Value val(d);
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Value, noq::ReferenceType::Any, val);
}

rParameter ::= TRUE. {
    noq::Value val(true);
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Value, noq::ReferenceType::Any, val);
}

rParameter ::= FALSE. {
    noq::Value val(false);
    s_ctx->parameter = std::make_unique<noq::Parameter>(noq::Parameter::Type::Value, noq::ReferenceType::Any, val);
}

/////////////////////////////////////////////
// Lexer
%lexer_integration ON.
//%lexer_debuglevel HIGH.

INSERT ::= "INSERT".
DOCUMENT ::= "DOCUMENT".
PAIR ::= "PAIR".

VERIFY ::= "VERIFY".
RULE ::= "RULE".

COLON_EQ ::= ":=".

NOT ::= "!".

AND ::= "&&".
OR ::= "\|\|".
ASSIGN ::= "=".
DEEP_EQ ::= "==".
SHALLOW_EQ ::= "~=".
NEQ ::= "!=".
LT ::= "<".
GT ::= ">".
LTE ::= "<=".
GTE ::= ">=".
PLUS ::= "\+".
MINUS ::= "-".
STAR ::= "\*".
DIVIDE ::= "/".
PERCENT ::= "%".

LBRACKET ::= "\(".
RBRACKET ::= "\)".
LCURLY ::= "\{".
RCURLY ::= "\}".
LSQUARE ::= "\[".
RSQUARE ::= "\]".
COMMA ::= ",".
SEMI ::= ";".
COLON ::= ":".

IDENTIFIER ::= "[\l][\l\d_]*".
VARIABLE    ::= !"@" "[\l][\l\d_]*".
AVARIABLE   ::= "@".
STRING  ::= !"\"" "([^\"]|(\\.))*" !"\"".

F_NUMBER ::= "[\d]+\.[\d]+".
F_NUMBER ::= "-[\d]+\.[\d]+".
I_NUMBER ::= "[\d]+".
I_NUMBER ::= "-[\d]+".

TRUE ::= "true".
FALSE ::= "false".
NULLV ::= "null".

WS ::= " ".
WS ::= "\t".
WS ::= "\r".
WS ::= "\n".
WS ::= "\r\n".
WS ::= "//.*\n".
WS ::= "//.*\r\n".

%code {

    void noq::Module::loadStreamModule(std::istream& is) {
        s_ctx = std::make_unique<Context>(this);
        int rc = ParseReadStream(is, "<string>", "DBG_PREFIX:");
        if(rc != 0){
            printf("Error\n");
            return;
        }
    }

    std::unique_ptr<noq::Function> noq::Query::parseStringQuery(const std::string& str) {
        s_ctx = std::make_unique<Context>(nullptr);
        int rc = ParseReadString((char*)str.c_str(), "<string>", "DBG_PREFIX:");
        if(rc != 0){
            printf("Error\n");
            return nullptr;
        }
        return std::move(s_ctx->function);
    }
}
