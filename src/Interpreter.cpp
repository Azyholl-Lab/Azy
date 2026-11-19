#include "Interpreter.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// ============================================================
// Helpers
// ============================================================
static bool valuesEqual(const Value& a, const Value& b) {
    if (a.kind == Value::Kind::Null && b.kind == Value::Kind::Null) return true;
    if (a.isNumber() && b.isNumber()) return a.asDouble() == b.asDouble();
    if (a.kind != b.kind) return false;
    switch (a.kind) {
        case Value::Kind::String: return a.strVal == b.strVal;
        case Value::Kind::Bool:   return a.boolVal == b.boolVal;
        case Value::Kind::Function: return a.funcVal == b.funcVal;
        default: return false;
    }
}

// В for-контексте `i == N` читается как `i != N` (спец. семантика Azy)
static bool isForSpecialCondition(const ExpressionNode* cond, const std::string& varName) {
    if (auto bin = dynamic_cast<const BinaryOpNode*>(cond)) {
        if (bin->op == "==") {
            auto lv = dynamic_cast<const VariableNode*>(bin->left.get());
            auto rv = dynamic_cast<const VariableNode*>(bin->right.get());
            if ((lv && lv->name == varName) || (rv && rv->name == varName)) return true;
        }
    }
    return false;
}

// ============================================================
// Interpreter
// ============================================================
Interpreter::Interpreter() : env(&globals) {}

void Interpreter::runtimeError(ErrCode code, SourcePos pos, const std::string& detail) {
    throw std::runtime_error(formatError(code, pos.line, pos.column, "", detail));
}

void Interpreter::execute(const ASTNode* root) {
    env = &globals;
    if (!root) return;

    auto block = dynamic_cast<const BlockNode*>(root);
    try {
        if (block) {
            for (const auto& stmt : block->statements) exec(stmt.get());
        } else {
            exec(root);
        }
    } catch (const ReturnSignal&) {
        // return на верхнем уровне — игнорируем (S003 должен был поймать)
    } catch (const BreakSignal&) {
    } catch (const ContinueSignal&) {
    }
}

// ============================================================
// Statements
// ============================================================
void Interpreter::exec(const ASTNode* node) {
    if (!node) return;

    // Expression used as statement → eval & discard
    if (auto expr = dynamic_cast<const ExpressionNode*>(node)) {
        eval(expr);
        return;
    }
    if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node)) {
        eval(exprStmt->expr.get());
        return;
    }

    if (auto block = dynamic_cast<const BlockNode*>(node)) {
        Environment local(env);
        Environment* saved = env;
        env = &local;
        try {
            for (const auto& stmt : block->statements) exec(stmt.get());
        } catch (...) { env = saved; throw; }
        env = saved;
        return;
    }

    if (auto varDecl = dynamic_cast<const VarDeclNode*>(node)) {
        for (const auto& d : varDecl->decls) {
            Value v = d.init ? eval(d.init.get()) : Value::null();
            env->define(d.name, std::move(v));
        }
        return;
    }

    if (auto fn = dynamic_cast<const FunctionNode*>(node)) {
        functions[fn->name] = fn;   // регистрация, тело не исполняем
        return;
    }

    if (auto ret = dynamic_cast<const ReturnNode*>(node)) {
        Value v = ret->expr ? eval(ret->expr.get()) : Value::null();
        throw ReturnSignal{std::move(v)};
    }

    if (dynamic_cast<const BreakNode*>(node))    throw BreakSignal{};
    if (dynamic_cast<const ContinueNode*>(node)) throw ContinueSignal{};

    if (auto assign = dynamic_cast<const AssignmentNode*>(node)) {
        Value rhs = eval(assign->expr.get());
        if (assign->op == "=") {
            if (!env->assign(assign->varName, std::move(rhs))) {
                runtimeError(ErrCode::S001_UndeclaredVariable, assign->pos,
                             "'" + assign->varName + "'");
            }
        } else {
            Value* current = env->lookup(assign->varName);
            if (!current) {
                runtimeError(ErrCode::S001_UndeclaredVariable, assign->pos,
                             "'" + assign->varName + "'");
            }
            std::string op = assign->op.substr(0, assign->op.size() - 1);  // "+=" → "+"
            Value newVal = applyBinary(op, *current, rhs, assign->pos);
            env->assign(assign->varName, std::move(newVal));
        }
        return;
    }

    if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        if (eval(ifNode->condition.get()).isTruthy()) exec(ifNode->thenBody.get());
        else if (ifNode->elseBody)                    exec(ifNode->elseBody.get());
        return;
    }

    if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        while (eval(whileNode->condition.get()).isTruthy()) {
            try { exec(whileNode->body.get()); }
            catch (const BreakSignal&) { break; }
            catch (const ContinueSignal&) { continue; }
        }
        return;
    }

    if (auto dw = dynamic_cast<const DoWhileNode*>(node)) {
        do {
            try { exec(dw->body.get()); }
            catch (const BreakSignal&) { break; }
            catch (const ContinueSignal&) { /* fall through */ }
        } while (eval(dw->condition.get()).isTruthy());
        return;
    }

    if (auto forNode = dynamic_cast<const ForNode*>(node)) {
        Environment loopEnv(env);
        Environment* saved = env;
        env = &loopEnv;

        try {
            if (forNode->isForeach) {
                Value iter = eval(forNode->iterable.get());
                if (iter.kind != Value::Kind::String) {
                    env = saved;
                    runtimeError(ErrCode::R003_RuntimeTypeError, forNode->varNamePos,
                                 "for-in работает только со строками, получен " + iter.typeName());
                }
                for (char c : iter.strVal) {
                    loopEnv.define(forNode->varName, Value::makeString(std::string(1, c)));
                    try { exec(forNode->body.get()); }
                    catch (const BreakSignal&) { break; }
                    catch (const ContinueSignal&) { continue; }
                }
            } else {
                // init
                if (forNode->init) {
                    if (auto assign = dynamic_cast<const AssignmentNode*>(forNode->init.get())) {
                        Value v = eval(assign->expr.get());
                        loopEnv.define(forNode->varName, std::move(v));
                    } else {
                        loopEnv.define(forNode->varName, Value::makeInt(0));
                        exec(forNode->init.get());
                    }
                } else {
                    loopEnv.define(forNode->varName, Value::makeInt(0));
                }

                // loop
                while (true) {
                    bool cont = true;
                    if (forNode->condition) {
                        if (isForSpecialCondition(forNode->condition.get(), forNode->varName)) {
                            auto bin = static_cast<const BinaryOpNode*>(forNode->condition.get());
                            Value l = eval(bin->left.get());
                            Value r = eval(bin->right.get());
                            cont = !valuesEqual(l, r);
                        } else {
                            cont = eval(forNode->condition.get()).isTruthy();
                        }
                    }
                    if (!cont) break;

                    try { exec(forNode->body.get()); }
                    catch (const BreakSignal&) { break; }
                    catch (const ContinueSignal&) { /* fall through */ }

                    // step
                    if (forNode->step) {
                        if (auto assign = dynamic_cast<const AssignmentNode*>(forNode->step.get())) {
                            Value v = eval(assign->expr.get());
                            env->assign(forNode->varName, std::move(v));
                        } else {
                            exec(forNode->step.get());
                        }
                    } else {
                        Value* v = env->lookup(forNode->varName);
                        if (v && v->kind == Value::Kind::Int) v->intVal++;
                        else if (v && v->kind == Value::Kind::Float) v->floatVal++;
                    }
                }
            }
        } catch (...) { env = saved; throw; }
        env = saved;
        return;
    }

    if (auto printNode = dynamic_cast<const PrintNode*>(node)) {
        Value v = eval(printNode->expr.get());
        std::cout << v.toString();
        if (printNode->newline) std::cout << "\n";
        return;
    }

    if (auto inputNode = dynamic_cast<const InputNode*>(node)) {
        for (const auto& var : inputNode->targetVars) {
            std::string s;
            if (inputNode->readLine) {
                if (!std::getline(std::cin, s))
                    runtimeError(ErrCode::R002_InputReadFailed, {0,0}, "для '" + var + "'");
            } else {
                if (!(std::cin >> s))
                    runtimeError(ErrCode::R002_InputReadFailed, {0,0}, "для '" + var + "'");
            }
            // Не нашли в цепочке scope'ов — неявно объявляем в текущем
            if (!env->assign(var, Value::makeString(s))) {
                env->define(var, Value::makeString(s));
            }
        }
        return;
    }

    runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "неизвестный узел AST");
}

// ============================================================
// Expressions
// ============================================================
Value Interpreter::eval(const ExpressionNode* node) {
    if (!node) return Value::null();

    if (auto lit = dynamic_cast<const LiteralNode*>(node)) {
        switch (lit->type) {
            case LiteralNode::Int:    return Value::makeInt(std::stoll(lit->value));
            case LiteralNode::Float:  return Value::makeFloat(std::stod(lit->value));
            case LiteralNode::String: return Value::makeString(lit->value);
            case LiteralNode::Bool:   return Value::makeBool(lit->value == "true");
        }
    }

    if (auto var = dynamic_cast<const VariableNode*>(node)) {
        Value* v = env->lookup(var->name);
        if (!v) runtimeError(ErrCode::S001_UndeclaredVariable, var->pos, "'" + var->name + "'");
        return *v;
    }

    if (auto call = dynamic_cast<const CallNode*>(node)) {
        std::vector<Value> args;
        for (const auto& a : call->args) args.push_back(eval(a.get()));

        if (isBuiltin(call->callee))
            return callBuiltin(call->callee, std::move(args));

        auto it = functions.find(call->callee);
        if (it == functions.end())
            runtimeError(ErrCode::R004_UndefinedFunction, call->pos, "'" + call->callee + "'");
        return callFunction(it->second, std::move(args));
    }

    if (auto bin = dynamic_cast<const BinaryOpNode*>(node)) {
        if (bin->op == "&&") {
            if (!eval(bin->left.get()).isTruthy()) return Value::makeBool(false);
            return Value::makeBool(eval(bin->right.get()).isTruthy());
        }
        if (bin->op == "||") {
            if (eval(bin->left.get()).isTruthy()) return Value::makeBool(true);
            return Value::makeBool(eval(bin->right.get()).isTruthy());
        }
        Value l = eval(bin->left.get());
        Value r = eval(bin->right.get());
        return applyBinary(bin->op, l, r, {0, 0});
    }

    if (auto un = dynamic_cast<const UnaryOpNode*>(node)) {
        if (un->op == "++_post" || un->op == "--_post") {
            auto var = dynamic_cast<const VariableNode*>(un->operand.get());
            if (!var) runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "++/-- только для переменных");
            Value* v = env->lookup(var->name);
            if (!v) runtimeError(ErrCode::S001_UndeclaredVariable, var->pos, "'" + var->name + "'");
            Value old = *v;
            if (v->kind == Value::Kind::Int)        v->intVal   += (un->op[0] == '+') ?  1 : -1;
            else if (v->kind == Value::Kind::Float) v->floatVal += (un->op[0] == '+') ?  1.0 : -1.0;
            else runtimeError(ErrCode::R003_RuntimeTypeError, var->pos, "++/-- только для чисел");
            return old;
        }
        Value v = eval(un->operand.get());
        return applyUnary(un->op, v, {0, 0});
    }

    if (auto inp = dynamic_cast<const InputExprNode*>(node)) {
        std::string s;
        if (inp->readLine) {
            if (!std::getline(std::cin, s)) runtimeError(ErrCode::R002_InputReadFailed, {0,0});
        } else {
            if (!(std::cin >> s)) runtimeError(ErrCode::R002_InputReadFailed, {0,0});
        }
        return Value::makeString(s);
    }

    runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "неизвестное выражение");
}

// ============================================================
// Binary / Unary
// ============================================================
Value Interpreter::applyBinary(const std::string& op, const Value& l, const Value& r, SourcePos pos) {
    if (op == "==") return Value::makeBool(valuesEqual(l, r));
    if (op == "!=") return Value::makeBool(!valuesEqual(l, r));

    if (op == "+") {
        if (l.kind == Value::Kind::String || r.kind == Value::Kind::String)
            return Value::makeString(l.toString() + r.toString());
        if (l.isNumber() && r.isNumber()) {
            if (l.kind == Value::Kind::Float || r.kind == Value::Kind::Float)
                return Value::makeFloat(l.asDouble() + r.asDouble());
            return Value::makeInt(l.intVal + r.intVal);
        }
        runtimeError(ErrCode::R003_RuntimeTypeError, pos,
                     "оператор '+' не применим к " + l.typeName() + " и " + r.typeName());
    }

    if (op == "-" || op == "*") {
        if (l.isNumber() && r.isNumber()) {
            if (l.kind == Value::Kind::Float || r.kind == Value::Kind::Float) {
                return Value::makeFloat(op == "-" ? l.asDouble() - r.asDouble() : l.asDouble() * r.asDouble());
            }
            return Value::makeInt(op == "-" ? l.intVal - r.intVal : l.intVal * r.intVal);
        }
        runtimeError(ErrCode::R003_RuntimeTypeError, pos,
                     "оператор '" + op + "' не применим к " + l.typeName() + " и " + r.typeName());
    }

    if (op == "/") {
        if (l.isNumber() && r.isNumber()) {
            if (r.asDouble() == 0.0) runtimeError(ErrCode::R001_DivisionByZero, pos);
            if (l.kind == Value::Kind::Float || r.kind == Value::Kind::Float)
                return Value::makeFloat(l.asDouble() / r.asDouble());
            return Value::makeInt(l.intVal / r.intVal);
        }
        runtimeError(ErrCode::R003_RuntimeTypeError, pos,
                     "оператор '/' не применим к " + l.typeName() + " и " + r.typeName());
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (l.isNumber() && r.isNumber()) {
            double lv = l.asDouble(), rv = r.asDouble();
            if (op == "<")  return Value::makeBool(lv <  rv);
            if (op == ">")  return Value::makeBool(lv >  rv);
            if (op == "<=") return Value::makeBool(lv <= rv);
            return Value::makeBool(lv >= rv);
        }
        if (l.kind == Value::Kind::String && r.kind == Value::Kind::String) {
            if (op == "<")  return Value::makeBool(l.strVal <  r.strVal);
            if (op == ">")  return Value::makeBool(l.strVal >  r.strVal);
            if (op == "<=") return Value::makeBool(l.strVal <= r.strVal);
            return Value::makeBool(l.strVal >= r.strVal);
        }
        runtimeError(ErrCode::R003_RuntimeTypeError, pos,
                     "оператор '" + op + "' не применим к " + l.typeName() + " и " + r.typeName());
    }

    runtimeError(ErrCode::R003_RuntimeTypeError, pos, "неизвестный бинарный оператор '" + op + "'");
}

Value Interpreter::applyUnary(const std::string& op, const Value& v, SourcePos pos) {
    if (op == "-") {
        if (v.kind == Value::Kind::Int)   return Value::makeInt(-v.intVal);
        if (v.kind == Value::Kind::Float) return Value::makeFloat(-v.floatVal);
        runtimeError(ErrCode::R003_RuntimeTypeError, pos, "унарный '-' для " + v.typeName());
    }
    if (op == "!") return Value::makeBool(!v.isTruthy());
    runtimeError(ErrCode::R003_RuntimeTypeError, pos, "неизвестный унарный оператор '" + op + "'");
}

// ============================================================
// Function calls
// ============================================================
Value Interpreter::callFunction(const FunctionNode* fn, std::vector<Value> args) {
    if (args.size() != fn->params.size()) {
        runtimeError(ErrCode::R003_RuntimeTypeError, fn->pos,
                     "функция '" + fn->name + "' ожидает " + std::to_string(fn->params.size()) +
                     " арг., передано " + std::to_string(args.size()));
    }

    Environment local(&globals);   // лексическая область: только глобалы (без замыканий)
    for (size_t i = 0; i < fn->params.size(); i++)
        local.define(fn->params[i], std::move(args[i]));

    Environment* saved = env;
    env = &local;
    try {
        exec(fn->body.get());
    } catch (ReturnSignal& sig) {
        env = saved;
        return std::move(sig.value);
    } catch (...) { env = saved; throw; }
    env = saved;
    return Value::null();
}

bool Interpreter::isBuiltin(const std::string& name) const {
    return name == "len" || name == "str" || name == "int" || name == "float";
}

Value Interpreter::callBuiltin(const std::string& name, std::vector<Value> args) {
    if (name == "len") {
        if (args.size() != 1) runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "len ожидает 1 арг.");
        if (args[0].kind == Value::Kind::String)
            return Value::makeInt((long long)args[0].strVal.size());
        runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "len не применим к " + args[0].typeName());
    }
    if (name == "str") {
        if (args.size() != 1) runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "str ожидает 1 арг.");
        return Value::makeString(args[0].toString());
    }
    if (name == "int") {
        if (args.size() != 1) runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "int ожидает 1 арг.");
        if (args[0].kind == Value::Kind::Int)   return args[0];
        if (args[0].kind == Value::Kind::Float) return Value::makeInt((long long)args[0].floatVal);
        if (args[0].kind == Value::Kind::Bool)  return Value::makeInt(args[0].boolVal ? 1 : 0);
        if (args[0].kind == Value::Kind::String) {
            try { return Value::makeInt(std::stoll(args[0].strVal)); }
            catch (...) { runtimeError(ErrCode::R003_RuntimeTypeError, {0,0},
                          "не удалось преобразовать '" + args[0].strVal + "' в int"); }
        }
    }
    if (name == "float") {
        if (args.size() != 1) runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "float ожидает 1 арг.");
        if (args[0].kind == Value::Kind::Float) return args[0];
        if (args[0].kind == Value::Kind::Int)   return Value::makeFloat((double)args[0].intVal);
        if (args[0].kind == Value::Kind::String) {
            try { return Value::makeFloat(std::stod(args[0].strVal)); }
            catch (...) { runtimeError(ErrCode::R003_RuntimeTypeError, {0,0},
                          "не удалось преобразовать '" + args[0].strVal + "' в float"); }
        }
    }
    runtimeError(ErrCode::R003_RuntimeTypeError, {0,0}, "неизвестная встроенная функция '" + name + "'");
}