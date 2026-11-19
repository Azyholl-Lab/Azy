#include "Semantic.h"

// ============================================================
// Встроенные функции, известные интерпретатору
// ============================================================
static const char* BUILTIN_FUNCTIONS[] = { "len", "str", "int", "float" };

// ============================================================
// Вспомогательные методы
// ============================================================
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

// ============================================================
// Точка входа
// ============================================================
std::vector<std::string> SemanticAnalyzer::analyze(const ASTNode* root) {
    errors.clear();
    scopes.clear();
    loopDepth = 0;
    functionDepth = 0;

    // Внешний scope — только builtin-функции.
    // Пользовательские объявления (var, fn) живут в дочернем scope,
    // созданном visit(BlockNode) для корневого блока.
    pushScope();
    for (const char* name : BUILTIN_FUNCTIONS) {
        scopes.back()[name] = { SemanticSymbol::Function, SourcePos{0, 0} };
    }

    visit(root);
    popScope();
    return errors;
}

// ============================================================
// Statements
// ============================================================
void SemanticAnalyzer::visit(const ASTNode* node) {
    if (!node) return;

    // --- Блок: новая область видимости ---
    if (auto block = dynamic_cast<const BlockNode*>(node)) {
        pushScope();
        for (const auto& stmt : block->statements) visit(stmt.get());
        popScope();
        return;
    }

    // --- var x = 5, y = 10; ---
    if (auto varDecl = dynamic_cast<const VarDeclNode*>(node)) {
        for (const auto& d : varDecl->decls) {
            if (d.init) visitExpr(d.init.get());
            declare(d.name, {SemanticSymbol::Var, d.pos}, d.pos);
        }
        return;
    }

    // --- fn name(params) { body } ---
    if (auto fn = dynamic_cast<const FunctionNode*>(node)) {
        declare(fn->name, {SemanticSymbol::Function, fn->pos}, fn->pos);
        functionDepth++;
        pushScope();
        for (const auto& p : fn->params) {
            declare(p, {SemanticSymbol::Param, fn->pos}, fn->pos);
        }
        visit(fn->body.get());
        popScope();
        functionDepth--;
        return;
    }

    // --- return expr; ---
    if (auto ret = dynamic_cast<const ReturnNode*>(node)) {
        if (functionDepth == 0)
            error(ErrCode::S003_ReturnOutsideFunction, ret->pos);
        if (ret->expr) visitExpr(ret->expr.get());
        return;
    }

    // --- break; ---
    if (auto br = dynamic_cast<const BreakNode*>(node)) {
        if (loopDepth == 0)
            error(ErrCode::S004_BreakOutsideLoop, br->pos);
        return;
    }

    // --- continue; ---
    if (auto cont = dynamic_cast<const ContinueNode*>(node)) {
        if (loopDepth == 0)
            error(ErrCode::S005_ContinueOutsideLoop, cont->pos);
        return;
    }

    // --- x = expr; / x += expr; / ... ---
    if (auto assign = dynamic_cast<const AssignmentNode*>(node)) {
        if (assign->expr) visitExpr(assign->expr.get());
        if (!lookup(assign->varName)) {
            error(ErrCode::S001_UndeclaredVariable, assign->pos,
                  "'" + assign->varName + "'");
        }
        return;
    }

    // --- if (cond) { ... } [else ...] ---
    if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        if (ifNode->condition) visitExpr(ifNode->condition.get());
        if (ifNode->thenBody) visit(ifNode->thenBody.get());
        if (ifNode->elseBody) visit(ifNode->elseBody.get());
        return;
    }

    // --- while (cond) { ... } ---
    if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        if (whileNode->condition) visitExpr(whileNode->condition.get());
        loopDepth++;
        if (whileNode->body) visit(whileNode->body.get());
        loopDepth--;
        return;
    }

    // --- do { ... } while (cond); ---
    if (auto doWhile = dynamic_cast<const DoWhileNode*>(node)) {
        loopDepth++;
        if (doWhile->body) visit(doWhile->body.get());
        loopDepth--;
        if (doWhile->condition) visitExpr(doWhile->condition.get());
        return;
    }

    // --- for (i = 0; i == 10; i++) / for (i == 5) / for (x in str) ---
    if (auto forNode = dynamic_cast<const ForNode*>(node)) {
        pushScope();
        // Переменная цикла живёт в собственном scope for
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
        return;
    }

    // --- print(expr); / print.ln(expr); ---
    if (auto printNode = dynamic_cast<const PrintNode*>(node)) {
        if (printNode->expr) visitExpr(printNode->expr.get());
        return;
    }

    // --- input(x, y, z); / input.line(x); ---
    // input(x) неявно объявляет x, если его ещё нет (out-параметр).
    if (auto inputNode = dynamic_cast<const InputNode*>(node)) {
        for (const auto& v : inputNode->targetVars) {
            if (!lookup(v)) {
                declare(v, {SemanticSymbol::Var, {0, 0}}, {0, 0});
            }
        }
        return;
    }

    // --- выражение как оператор: greet(x); ---
    if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node)) {
        if (exprStmt->expr) visitExpr(exprStmt->expr.get());
        return;
    }
}

// ============================================================
// Expressions
// ============================================================
void SemanticAnalyzer::visitExpr(const ExpressionNode* node) {
    if (!node) return;

    // --- переменная ---
    if (auto var = dynamic_cast<const VariableNode*>(node)) {
        if (!lookup(var->name)) {
            error(ErrCode::S001_UndeclaredVariable, var->pos,
                  "'" + var->name + "'");
        }
        return;
    }

    // --- вызов функции: callee(arg1, arg2, ...) ---
    if (auto call = dynamic_cast<const CallNode*>(node)) {
        if (!lookup(call->callee)) {
            error(ErrCode::S001_UndeclaredVariable, call->pos,
                  "'" + call->callee + "'");
        }
        for (const auto& arg : call->args) visitExpr(arg.get());
        return;
    }

    // --- бинарная операция ---
    if (auto bin = dynamic_cast<const BinaryOpNode*>(node)) {
        if (bin->left) visitExpr(bin->left.get());
        if (bin->right) visitExpr(bin->right.get());
        return;
    }

    // --- унарная операция (!expr, -expr, not expr, i++, i--) ---
    if (auto un = dynamic_cast<const UnaryOpNode*>(node)) {
        if (un->operand) visitExpr(un->operand.get());
        return;
    }

    // --- input() / input.line() как выражение ---
    if (dynamic_cast<const InputExprNode*>(node)) {
        return;  // проверять нечего
    }

    // --- литералы ---
    if (dynamic_cast<const LiteralNode*>(node)) {
        return;  // проверять нечего
    }

    // Всё остальное — неизвестный ExpressionNode (не должно случаться)
}