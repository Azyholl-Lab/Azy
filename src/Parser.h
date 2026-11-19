#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <memory>
#include <iostream>
#include "Token.h"
#include "Errors.h"

// ============================================================
// SourcePos — лёгкая позиция в исходнике для AST-узлов
// ============================================================
struct SourcePos {
    int line = 0;
    int column = 0;
    SourcePos() = default;
    SourcePos(int l, int c) : line(l), column(c) {}
    SourcePos(const Token& t) : line(t.line), column(t.column) {}
};

// ============================================================
// Базовые узлы AST
// ============================================================
struct ASTNode {
    virtual ~ASTNode() = default;
};

struct ExpressionNode : ASTNode {};

struct ExpressionStatementNode : ASTNode {
    std::unique_ptr<ExpressionNode> expr;
    ExpressionStatementNode(std::unique_ptr<ExpressionNode> e)
        : expr(std::move(e)) {}
};

struct BlockNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
};

struct AssignmentNode : ASTNode {
    std::string varName;
    std::string op;
    std::unique_ptr<ExpressionNode> expr;
    SourcePos pos;
    AssignmentNode(std::string name, std::string operation,
                   std::unique_ptr<ExpressionNode> e, SourcePos p = {})
        : varName(std::move(name)), op(std::move(operation)),
          expr(std::move(e)), pos(p) {}
};

struct IfNode : ASTNode {
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> thenBody;
    std::unique_ptr<ASTNode> elseBody;
    IfNode(std::unique_ptr<ExpressionNode> cond,
           std::unique_ptr<ASTNode> thenB,
           std::unique_ptr<ASTNode> elseB = nullptr)
        : condition(std::move(cond)), thenBody(std::move(thenB)), elseBody(std::move(elseB)) {}
};

struct WhileNode : ASTNode {
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> body;
    WhileNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<ASTNode> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

struct DoWhileNode : ASTNode {
    std::unique_ptr<ASTNode> body;
    std::unique_ptr<ExpressionNode> condition;
    DoWhileNode(std::unique_ptr<ASTNode> b, std::unique_ptr<ExpressionNode> cond)
        : body(std::move(b)), condition(std::move(cond)) {}
};

struct ForNode : ASTNode {
    std::string varName;
    SourcePos varNamePos;
    std::unique_ptr<ASTNode> init;
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ASTNode> step;
    std::unique_ptr<ExpressionNode> iterable;
    bool isForeach = false;
    std::unique_ptr<ASTNode> body;
    ForNode() = default;
};

struct BreakNode : ASTNode {
    SourcePos pos;
    BreakNode(SourcePos p = {}) : pos(p) {}
};

struct ContinueNode : ASTNode {
    SourcePos pos;
    ContinueNode(SourcePos p = {}) : pos(p) {}
};

struct PrintNode : ASTNode {
    std::unique_ptr<ExpressionNode> expr;
    bool newline;
    PrintNode(std::unique_ptr<ExpressionNode> e, bool nl)
        : expr(std::move(e)), newline(nl) {}
};

struct InputNode : ASTNode {
    std::vector<std::string> targetVars;
    bool readLine;
    InputNode(std::vector<std::string> vars, bool rl)
        : targetVars(std::move(vars)), readLine(rl) {}
};

struct InputExprNode : ExpressionNode {
    bool readLine;
    InputExprNode(bool rl) : readLine(rl) {}
};

// ============================================================
// Объявления переменных и функций, return
// ============================================================
struct VarDeclNode : ASTNode {
    struct Decl {
        std::string name;
        std::unique_ptr<ExpressionNode> init;
        SourcePos pos;
        Decl(std::string n, std::unique_ptr<ExpressionNode> i, SourcePos p = {})
            : name(std::move(n)), init(std::move(i)), pos(p) {}
        Decl(Decl&&) = default;
        Decl& operator=(Decl&&) = default;
        Decl(const Decl&) = delete;
        Decl& operator=(const Decl&) = delete;
    };
    std::vector<Decl> decls;
    VarDeclNode() = default;
};

struct FunctionNode : ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<ASTNode> body;
    SourcePos pos;
    FunctionNode(std::string n, std::vector<std::string> p,
                 std::unique_ptr<ASTNode> b, SourcePos ps = {})
        : name(std::move(n)), params(std::move(p)),
          body(std::move(b)), pos(ps) {}
};

struct ReturnNode : ASTNode {
    std::unique_ptr<ExpressionNode> expr;
    SourcePos pos;
    ReturnNode(std::unique_ptr<ExpressionNode> e, SourcePos p = {})
        : expr(std::move(e)), pos(p) {}
};

// ============================================================
// Узлы выражений
// ============================================================
struct LiteralNode : ExpressionNode {
    enum Type { Int, Float, String, Bool };
    Type type;
    std::string value;
    LiteralNode(Type t, const std::string& val) : type(t), value(val) {}
};

struct VariableNode : ExpressionNode {
    std::string name;
    SourcePos pos;
    VariableNode(const std::string& n, SourcePos p = {})
        : name(n), pos(p) {}
};

// ⭐ НОВЫЙ УЗЕЛ: вызов функции
struct CallNode : ExpressionNode {
    std::string callee;
    SourcePos pos;
    std::vector<std::unique_ptr<ExpressionNode>> args;
    CallNode(std::string c, SourcePos p) : callee(std::move(c)), pos(p) {}
};

struct UnaryOpNode : ExpressionNode {
    std::string op;
    std::unique_ptr<ExpressionNode> operand;
    UnaryOpNode(const std::string& o, std::unique_ptr<ExpressionNode> opnd)
        : op(o), operand(std::move(opnd)) {}
};

struct BinaryOpNode : ExpressionNode {
    std::string op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    BinaryOpNode(const std::string& o,
                 std::unique_ptr<ExpressionNode> l,
                 std::unique_ptr<ExpressionNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

// ============================================================
// Класс парсера
// ============================================================
class Parser {
private:
    std::vector<Token> tokens;
    size_t currentPos = 0;

    Token peek();
    Token peekNext();
    Token consume();
    bool check(TokenType type);
    bool check(TokenType type, const std::string& value);
    bool checkNext(TokenType type);
    bool checkNext(TokenType type, const std::string& value);

    void debugging(bool debug = false) {
        if (debug) {
            Token token = peek();
            std::cout << "[Парсер] " << token.value << "\n";
        }
    }

    void Close_block();

public:
    Parser(const std::vector<Token>& toks);

    void printAST(const ASTNode* node, int indent = 0) {
        std::string pad(indent * 2, ' ');

        if (auto block = dynamic_cast<const BlockNode*>(node)) {
            std::cout << pad << "Block {\n";
            for (const auto& stmt : block->statements) {
                printAST(stmt.get(), indent + 1);
            }
            std::cout << pad << "}\n";
        }
        else if (auto assign = dynamic_cast<const AssignmentNode*>(node)) {
            std::cout << pad << "Assignment: " << assign->varName
                    << " " << assign->op << " ";
            printAST(assign->expr.get(), 0);
            std::cout << "\n";
        }
        else if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
            std::cout << pad << "If (";
            printAST(ifNode->condition.get(), 0);
            std::cout << ") {\n";
            printAST(ifNode->thenBody.get(), indent + 1);
            if (ifNode->elseBody) {
                std::cout << pad << "} else {\n";
                printAST(ifNode->elseBody.get(), indent + 1);
            }
            std::cout << pad << "}\n";
        }
        else if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
            std::cout << pad << "While (";
            printAST(whileNode->condition.get(), 0);
            std::cout << ") {\n";
            printAST(whileNode->body.get(), indent + 1);
            std::cout << pad << "}\n";
        }
        else if (auto doWhile = dynamic_cast<const DoWhileNode*>(node)) {
            std::cout << pad << "Do {\n";
            printAST(doWhile->body.get(), indent + 1);
            std::cout << pad << "} While (";
            printAST(doWhile->condition.get(), 0);
            std::cout << ")\n";
        }
        else if (auto forNode = dynamic_cast<const ForNode*>(node)) {
            if (forNode->isForeach) {
                std::cout << pad << "For (" << forNode->varName << " in ";
                printAST(forNode->iterable.get(), 0);
                std::cout << ") {\n";
            } else {
                std::cout << pad << "For (" << forNode->varName << ":\n";
                std::cout << pad << "  init: ";
                if (forNode->init) printAST(forNode->init.get(), 0);
                else std::cout << forNode->varName << " = 0\n";
                std::cout << pad << "  cond: ";
                if (forNode->condition) printAST(forNode->condition.get(), 0);
                else std::cout << "null";
                std::cout << "\n";
                std::cout << pad << "  step: ";
                if (forNode->step) {
                    printAST(forNode->step.get(), 0);
                    if (!dynamic_cast<const AssignmentNode*>(forNode->step.get()))
                        std::cout << "\n";
                } else {
                    std::cout << forNode->varName << "++\n";
                }
                std::cout << pad << ") {\n";
            }
            printAST(forNode->body.get(), indent + 1);
            std::cout << pad << "}\n";
        }
        else if (dynamic_cast<const BreakNode*>(node)) {
            std::cout << pad << "Break\n";
        }
        else if (dynamic_cast<const ContinueNode*>(node)) {
            std::cout << pad << "Continue\n";
        }
        else if (auto printNode = dynamic_cast<const PrintNode*>(node)) {
            std::cout << pad << "Print" << (printNode->newline ? ".ln" : "") << " (";
            printAST(printNode->expr.get(), 0);
            std::cout << ")\n";
        }
        else if (auto inputNode = dynamic_cast<const InputNode*>(node)) {
            std::cout << pad << "Input" << (inputNode->readLine ? ".line" : "") << "(";
            for (size_t i = 0; i < inputNode->targetVars.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << "-> " << inputNode->targetVars[i];
            }
            std::cout << ")\n";
        }
        else if (auto inputExpr = dynamic_cast<const InputExprNode*>(node)) {
            std::cout << pad << "Input" << (inputExpr->readLine ? ".line" : "") << "()";
        }
        else if (auto varDecl = dynamic_cast<const VarDeclNode*>(node)) {
            std::cout << pad << "VarDecl {\n";
            for (const auto& d : varDecl->decls) {
                std::cout << pad << "  " << d.name;
                if (d.init) {
                    std::cout << " = ";
                    printAST(d.init.get(), 0);
                } else {
                    std::cout << " (uninitialized)";
                }
                std::cout << "\n";
            }
            std::cout << pad << "}\n";
        }
        else if (auto fn = dynamic_cast<const FunctionNode*>(node)) {
            std::cout << pad << "Function " << fn->name << "(";
            for (size_t i = 0; i < fn->params.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << fn->params[i];
            }
            std::cout << ") {\n";
            if (fn->body) printAST(fn->body.get(), indent + 1);
            std::cout << pad << "}\n";
        }
        else if (auto ret = dynamic_cast<const ReturnNode*>(node)) {
            std::cout << pad << "Return";
            if (ret->expr) {
                std::cout << " ";
                printAST(ret->expr.get(), 0);
            }
            std::cout << "\n";
        }
        else if (auto literal = dynamic_cast<const LiteralNode*>(node)) {
            std::cout << pad << "Literal(";
            switch (literal->type) {
                case LiteralNode::Int:   std::cout << "int"; break;
                case LiteralNode::Float: std::cout << "float"; break;
                case LiteralNode::String:std::cout << "string"; break;
                case LiteralNode::Bool:  std::cout << "bool"; break;
            }
            std::cout << ", \"" << literal->value << "\")";
        }
        else if (auto var = dynamic_cast<const VariableNode*>(node)) {
            std::cout << pad << "Variable(\"" << var->name << "\")";
        }
        else if (auto call = dynamic_cast<const CallNode*>(node)) {
            std::cout << pad << "Call " << call->callee << "(";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) std::cout << ", ";
                printAST(call->args[i].get(), 0);
            }
            std::cout << ")";
        }
        else if (auto unary = dynamic_cast<const UnaryOpNode*>(node)) {
            std::cout << pad << "UnaryOp(\"" << unary->op << "\", ";
            printAST(unary->operand.get(), 0);
            std::cout << ")";
        }
        else if (auto binary = dynamic_cast<const BinaryOpNode*>(node)) {
            std::cout << pad << "BinaryOp(\"" << binary->op << "\", ";
            printAST(binary->left.get(), 0);
            std::cout << ", ";
            printAST(binary->right.get(), 0);
            std::cout << ")";
        }
        else if (auto exprStmt = dynamic_cast<const ExpressionStatementNode*>(node)) {
            printAST(exprStmt->expr.get(), indent);
            std::cout << "\n";
        }
        else {
            std::cout << pad << "Unknown node\n";
        }
    }

    std::unique_ptr<ASTNode> parse();

    template <typename... Args>
    void parserError(ErrCode code, const Token& token, Args&&... args) {
        std::ostringstream msgStream;
        (msgStream << ... << std::forward<Args>(args));
        throw std::runtime_error(formatError(code, token, msgStream.str()));
    }

    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseBlock();

    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseElse();

    std::unique_ptr<ASTNode> parseWhile();
    std::unique_ptr<ASTNode> parseDoWhile();

    std::unique_ptr<ASTNode> parseFor();
    std::unique_ptr<ASTNode> parseForInit();
    std::unique_ptr<ASTNode> parseForStep();
    std::unique_ptr<ASTNode> parseForBody();

    std::unique_ptr<ASTNode> parseBreak();
    std::unique_ptr<ASTNode> parseContinue();

    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parsePostfixStatement();
    std::unique_ptr<ASTNode> parseExpressionStatement();

    std::unique_ptr<ASTNode> parsePrint();
    std::unique_ptr<ASTNode> parseInput();
    std::unique_ptr<ExpressionNode> parseInputExpression();

    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseFunction();
    std::unique_ptr<ASTNode> parseReturn();

    std::unique_ptr<ExpressionNode> parseLogicalOr();
    std::unique_ptr<ExpressionNode> parseLogicalAnd();
    std::unique_ptr<ExpressionNode> parseEquality();
    std::unique_ptr<ExpressionNode> parseRelational();
    std::unique_ptr<ExpressionNode> parseAdditive();
    std::unique_ptr<ExpressionNode> parseMultiplicative();
    std::unique_ptr<ExpressionNode> parseUnary();
    std::unique_ptr<ExpressionNode> parsePrimary();

    std::unique_ptr<ExpressionNode> parseCall();   // ⭐ новое
};