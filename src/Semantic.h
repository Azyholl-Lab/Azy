#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Parser.h"
#include "Errors.h"

struct SemanticSymbol {
    enum Kind { Var, Param, Function };
    Kind kind;
    SourcePos pos;
};

class SemanticAnalyzer {
public:
    std::vector<std::string> analyze(const ASTNode* root);

private:
    std::vector<std::unordered_map<std::string, SemanticSymbol>> scopes;
    int loopDepth = 0;
    int functionDepth = 0;
    std::vector<std::string> errors;

    void pushScope() { scopes.emplace_back(); }
    void popScope()  { scopes.pop_back(); }

    SemanticSymbol* lookup(const std::string& name);
    void declare(const std::string& name, SemanticSymbol sym, SourcePos pos);
    void error(ErrCode code, SourcePos pos, const std::string& detail = "");

    void visit(const ASTNode* node);
    void visitExpr(const ExpressionNode* node);
};