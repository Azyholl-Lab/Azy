#include "Semantic.h"

SemanticSymbol* SemanticAnalyzer::lookup(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

void SemanticAnalyzer::declare(const std::string& name,
                               SemanticSymbol sym, SourcePos pos) {
    if (scopes.empty()) pushScope();
    auto& current = scopes.back();
    if (current.count(name)) {
        error(ErrCode::S002_DuplicateDeclaration, pos, "'" + name + "'");
        return;
    }
    current[name] = sym;
}

void SemanticAnalyzer::error(ErrCode code, SourcePos pos,
                             const std::string& detail) {
    errors.push_back(formatError(code, pos.line, pos.column, "", detail));
}

std::vector<std::string> SemanticAnalyzer::analyze(const ASTNode* root) {
    errors.clear();
    scopes.clear();
    loopDepth = 0;
    functionDepth = 0;
    visit(root);
    return errors;
}

// ============================================================
// Statements
// ============================================================
void SemanticAnalyzer::visit(const ASTNode* node) {
    if (!node) return;

    if (auto block = dynamic_cast<const BlockNode*>(node)) {
        pushScope();
        for (const auto& stmt : block->statements) visit(stmt.get());
        popScope();
    }
    else if (auto varDecl = dynamic_cast<const VarDeclNode*>(node)) {
        for (const auto& d : varDecl->decls) {
            if (d.init) visitExpr(d.init.get());
            declare(d.name, {SemanticSymbol::Var, d.pos}, d.pos);
        }
    }
    else if (auto fn = dynamic_cast<const FunctionNode*>(node)) {
        declare(fn->name, {SemanticSymbol::Function, fn->pos}, fn->pos);
        functionDepth++;
        pushScope();
        for (const auto& p : fn->params) {
            declare(p, {SemanticSymbol::Param, fn->pos}, fn->pos);
        }
        visit(fn->body.get());
        popScope();
        functionDepth--;
    }
    else if (auto ret = dynamic_cast<const ReturnNode*>(node)) {
        if (functionDepth == 0) error(ErrCode::S003_ReturnOutsideFunction, ret->pos);
        if (ret->expr) visitExpr(ret->expr.get());
    }
    else if (auto br = dynamic_cast<const BreakNode*>(node)) {
        if (loopDepth == 0) error(ErrCode::S004_BreakOutsideLoop, br->pos);
    }
    else if (auto cont = dynamic_cast<const ContinueNode*>(node)) {
        if (loopDepth == 0) error(ErrCode::S005_ContinueOutsideLoop, cont->pos);
    }
    else if (auto assign = dynamic_cast<const AssignmentNode*>(node)) {
        if (assign->expr) visitExpr(assign->expr.get());
        if (!lookup(assign->varName)) {
            error(ErrCode::S001_UndeclaredVariable, assign->pos,
                  "'" + assign->varName + "'");
        }
    }
    else if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        if (ifNode->condition) visitExpr(ifNode->condition.get());
        if (ifNode->thenBody) visit(ifNode->thenBody.get());
        if (ifNode->elseBody) visit(ifNode->elseBody.get());
    }
    else if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        if (whileNode->condition) visitExpr(whileNode->condition.get());
        loopDepth++;
        if (whileNode->body) visit(whileNode->body.get());
        loopDepth--;
    }
    else if (auto doWhile = dynamic_cast<const DoWhileNode*>(node)) {
        loopDepth++;
        if (doWhile->body) visit(doWhile->body.get());
        loopDepth--;
        if (doWhile->condition) visitExpr(doWhile->condition.get());
    }
    else if (auto forNode = dynamic_cast<const ForNode*>(node)) {
        pushScope();
        declare(forNode->varName,
                {SemanticSymbol::Var, forNode->varNamePos},
                forNode->varNamePos);

        if (forNode->isForeach) {
            if (forNode->iterable) visitExpr(forNode->iterable.get());
            loopDepth++;
            if (forNode->body) visit(forNode->body.get());
            loopDepth--;
        } else {
            if (forNode->init) visit(forNode->init.get());
            if (forNode->condition) visitExpr(forNode->condition.get());
            if (forNode->step) visit(forNode->step.get());
            loopDepth++;
            if (forNode->body) visit(forNode->body.get());
            loopDepth--;
        }
        popScope();
    }
    else if (auto printNode = dynamic_cast<const PrintNode*>(node)) {
        if (printNode->expr) visitExpr(printNode->expr.get());
    }
    else if (auto inputNode = dynamic_cast<const InputNode*>(node)) {
        for (const auto& v : inputNode->targetVars) {
            if (!lookup(v)) {
                error(ErrCode::S001_UndeclaredVariable, {0, 0}, "'" + v + "'");
            }
        }
    }
    else if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node)) {
        if (exprStmt->expr) visitExpr(exprStmt->expr.get());
    }
}

// ============================================================
// Expressions
// ============================================================
void SemanticAnalyzer::visitExpr(const ExpressionNode* node) {
    if (!node) return;

    if (auto var = dynamic_cast<const VariableNode*>(node)) {
        if (!lookup(var->name)) {
            error(ErrCode::S001_UndeclaredVariable, var->pos, "'" + var->name + "'");
        }
    }
    else if (auto call = dynamic_cast<const CallNode*>(node)) {
        if (!lookup(call->callee)) {
            error(ErrCode::S001_UndeclaredVariable, call->pos, "'" + call->callee + "'");
        }
        for (const auto& arg : call->args) visitExpr(arg.get());
    }
    else if (auto bin = dynamic_cast<const BinaryOpNode*>(node)) {
        if (bin->left) visitExpr(bin->left.get());
        if (bin->right) visitExpr(bin->right.get());
    }
    else if (auto un = dynamic_cast<const UnaryOpNode*>(node)) {
        if (un->operand) visitExpr(un->operand.get());
    }
    // Literal, InputExpr — проверять нечего
}