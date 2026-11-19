#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Parser.h"
#include "Value.h"
#include "Environment.h"
#include "Errors.h"

class Interpreter {
public:
    Interpreter();
    void execute(const ASTNode* root);

private:
    Environment globals;
    Environment* env;

    std::unordered_map<std::string, const FunctionNode*> functions;

    struct ReturnSignal   { Value value; };
    struct BreakSignal    {};
    struct ContinueSignal {};

    void  exec(const ASTNode* node);
    Value eval(const ExpressionNode* node);

    Value callFunction(const FunctionNode* fn, std::vector<Value> args);
    Value callBuiltin(const std::string& name, std::vector<Value> args);
    bool  isBuiltin(const std::string& name) const;

    Value applyBinary(const std::string& op, const Value& l, const Value& r, SourcePos pos);
    Value applyUnary(const std::string& op, const Value& v, SourcePos pos);

    [[noreturn]] void runtimeError(ErrCode code, SourcePos pos, const std::string& detail = "");
};