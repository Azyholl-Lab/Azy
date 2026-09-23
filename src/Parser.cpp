#include "Parser.h"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

// ============================================================
// Реализация Parser
// ============================================================

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), currentPos(0) {}

Token Parser::peek() {
    if (currentPos < tokens.size()) {
        return tokens[currentPos];
    }
    return {TokenType::EndOfFile, "", -1, -1};
}

Token Parser::peekNext() {
    if (currentPos + 1 < tokens.size()) {
        return tokens[currentPos + 1];
    }
    return {TokenType::EndOfFile, "", -1, -1};
}

Token Parser::consume() {
    Token tok = peek();
    if (currentPos < tokens.size()) {
        currentPos++;
    }
    return tok;
}

bool Parser::check(TokenType type) {
    return peek().type == type;
}

bool Parser::check(TokenType type, const std::string& value) {
    return peek().type == type && peek().value == value;
}

bool Parser::checkNext(TokenType type) {
    return peekNext().type == type;
}

bool Parser::checkNext(TokenType type, const std::string& value) {
    return peekNext().type == type && peekNext().value == value;
}

void Parser::Close_block() {
    if (check(TokenType::Semicolon)) {
        consume();
    } else {
        parserError(peek(),
            "Синтаксическая ошибка: ожидался терминатор ';', получен '" +
            peek().value + "'");
    }
}

// ============================================================
// Главный цикл разбора – возвращает корневой блок
// ============================================================
std::unique_ptr<ASTNode> Parser::parse() {
    auto block = std::make_unique<BlockNode>();
    while (currentPos < tokens.size() && peek().type != TokenType::EndOfFile) {
        size_t before = currentPos;
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        } else if (currentPos == before) {
            consume();   // защита от зацикливания
        }
    }
    return block;
}

// ============================================================
// Разбор операторов (возвращают узел AST)
// ============================================================
std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (check(TokenType::KwIf)) {
        return parseIf();
    } else if (check(TokenType::KwDo)) {
        return parseDoWhile();
    } else if (check(TokenType::KwWhile)) {
        return parseWhile();
    } else if (check(TokenType::KwFor)) {
        return parseFor();
    } else if (check(TokenType::KwBreak)) {
        return parseBreak();
    } else if (check(TokenType::KwContinue)) {
        return parseContinue();
    } else if (check(TokenType::Identifier, "print")) {
        return parsePrint();

    } else if (peek().type == TokenType::Identifier &&
            (checkNext(TokenType::Assign)      ||
             checkNext(TokenType::PlusAssign)  ||
             checkNext(TokenType::MinusAssign) ||
             checkNext(TokenType::MultAssign)  ||
             checkNext(TokenType::DivAssign))) {
        return parseAssignment();

    } else if (peek().type == TokenType::Identifier &&
            (checkNext(TokenType::Increment) ||
             checkNext(TokenType::Decrement))) {
        return parsePostfixStatement();

    } else {
        std::cerr << "[Парсер] Пропущен неожиданный токен: '"
                  << peek().value << "' (строка " << peek().line << ")\n";
        consume();
        return nullptr;
    }
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    debugging();
    consume(); // '{'

    auto block = std::make_unique<BlockNode>();
    while (peek().type != TokenType::RBrace &&
           peek().type != TokenType::EndOfFile) {
        size_t before = currentPos;
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        } else if (currentPos == before) {
            consume();
        }
    }

    if (peek().type != TokenType::RBrace) {
        parserError(peek(), "Ожидалась закрывающая фигурная скобка '}'");
    }
    debugging();
    consume(); // '}'
    return block;
}

// ============================================================
// Конкретные операторы
// ============================================================
std::unique_ptr<ASTNode> Parser::parseDoWhile() {
    debugging();
    consume(); // "do"

    if (peek().type != TokenType::LBrace) {
        parserError(peek(), "Ожидался блок кода '{...}' после 'do'");
        return nullptr;
    }
    auto body = parseBlock();

    if (!check(TokenType::KwWhile)) {
        parserError(peek(), "Ожидался идентификатор 'while' после блока кода");
        return nullptr;
    }
    consume(); // "while"

    if (peek().type != TokenType::LParen) {
        parserError(peek(), "Ожидалась открывающая скобка '(' после 'while'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
        return nullptr;
    }
    consume(); // ')'
    Close_block();
    return std::make_unique<DoWhileNode>(std::move(body), std::move(condition));
}

std::unique_ptr<ASTNode> Parser::parseWhile() {
    debugging();
    consume(); // "while"

    if (peek().type != TokenType::LParen) {
        parserError(peek(), "Ожидалась открывающая скобка '(' после 'while'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
        return nullptr;
    }
    consume(); // ')'

    if (peek().type != TokenType::LBrace) {
        parserError(peek(), "Ожидался блок кода '{...}' после условия");
        return nullptr;
    }
    auto body = parseBlock();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

// ============================================================
// Цикл for: три формы
//   for (i == x)               — сокращённая
//   for (i = 0; i == x; i++)   — полная
//   for (i in (int/string/array)) — foreach
// ============================================================
std::unique_ptr<ASTNode> Parser::parseFor() {
    debugging();
    consume(); // "for"

    if (peek().type != TokenType::LParen) {
        parserError(peek(), "Ожидалась '(' после 'for'");
        return nullptr;
    }
    consume(); // '('

    if (peek().type != TokenType::Identifier) {
        parserError(peek(), "Ожидался идентификатор переменной цикла");
        return nullptr;
    }

    std::string varName = peek().value;
    Token next = peekNext();

    // ================== ФОРМА 3: for (i in ...) ==================
    if (next.type == TokenType::KwIn) {
        consume(); // i
        consume(); // in

        auto iterable = parseLogicalOr();
        if (!iterable) return nullptr;

        if (peek().type != TokenType::RParen) {
            parserError(peek(), "Ожидалась ')' после for-in");
            return nullptr;
        }
        consume(); // ')'

        auto body = parseForBody();
        if (!body) return nullptr;

        auto node = std::make_unique<ForNode>();
        node->varName   = varName;
        node->iterable  = std::move(iterable);
        node->isForeach = true;
        node->body      = std::move(body);
        return node;
    }

    // ================== ФОРМА 1: for (i == x) ==================
    if (next.type == TokenType::Equal) {
        // НЕ consume() — parseLogicalOr() сам разберёт "i == 5"
        auto cond = parseLogicalOr();
        if (!cond) return nullptr;

        if (peek().type != TokenType::RParen) {
            parserError(peek(), "Ожидалась ')' после условия for");
            return nullptr;
        }
        consume(); // ')'

        auto body = parseForBody();
        if (!body) return nullptr;

        auto node = std::make_unique<ForNode>();
        node->varName   = varName;
        node->init      = nullptr;   // по умолчанию i = 0
        node->condition = std::move(cond);
        node->step      = nullptr;   // по умолчанию i++
        node->body      = std::move(body);
        return node;
    }

    // ================== ФОРМА 2: for (i = 0; i == x; i++) ==================
    if (next.type == TokenType::Assign) {
        auto init = parseForInit();
        if (!init) return nullptr;

        if (peek().type != TokenType::Semicolon) {
            parserError(peek(), "Ожидался ';' после инициализации for");
            return nullptr;
        }
        consume(); // ';'

        auto cond = parseLogicalOr();
        if (!cond) return nullptr;

        if (peek().type != TokenType::Semicolon) {
            parserError(peek(), "Ожидался ';' после условия for");
            return nullptr;
        }
        consume(); // ';'

        auto step = parseForStep();
        if (!step) return nullptr;

        if (peek().type != TokenType::RParen) {
            parserError(peek(), "Ожидалась ')' после шага for");
            return nullptr;
        }
        consume(); // ')'

        auto body = parseForBody();
        if (!body) return nullptr;

        auto node = std::make_unique<ForNode>();
        node->varName   = varName;
        node->init      = std::move(init);
        node->condition = std::move(cond);
        node->step      = std::move(step);
        node->body      = std::move(body);
        return node;
    }

    parserError(peek(),
        "Неизвестная форма for: ожидалось '=', '==' или 'in' после переменной");
    return nullptr;
}

// Отдельный оператор: i++;  или  i--;
std::unique_ptr<ASTNode> Parser::parsePostfixStatement() {
    Token name = consume();
    std::string op = consume().value;   // "++" или "--"
    auto var = std::make_unique<VariableNode>(name.value);
    Close_block();
    auto unary = std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
    return std::make_unique<ExpressionStatementNode>(std::move(unary));
}

// init в полной форме: i = <expr>
std::unique_ptr<ASTNode> Parser::parseForInit() {
    if (peek().type != TokenType::Identifier) {
        parserError(peek(), "Ожидался идентификатор в инициализации for");
        return nullptr;
    }
    std::string name = consume().value; // i

    if (!check(TokenType::Assign)) {
        parserError(peek(), "Ожидался '=' в инициализации for");
        return nullptr;
    }
    consume(); // '='
    auto expr = parseLogicalOr();
    return std::make_unique<AssignmentNode>(name, "=", std::move(expr));
}

// step в полной форме: i++ / i-- / i = expr
std::unique_ptr<ASTNode> Parser::parseForStep() {
    if (peek().type != TokenType::Identifier) {
        parserError(peek(), "Ожидался идентификатор в шаге for");
        return nullptr;
    }
    std::string name = consume().value; // i

    // i++ / i--
    if (peek().type == TokenType::Increment || peek().type == TokenType::Decrement) {
        std::string op = consume().value;
        auto var = std::make_unique<VariableNode>(name);
        return std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
    }

    // i = expr
    if (check(TokenType::Assign)) {
        consume();
        auto expr = parseLogicalOr();
        return std::make_unique<AssignmentNode>(name, "=", std::move(expr));
    }

    parserError(peek(), "Ожидался '++', '--' или '=' в шаге for");
    return nullptr;
}

// тело цикла — блок { ... }
std::unique_ptr<ASTNode> Parser::parseForBody() {
    if (peek().type != TokenType::LBrace) {
        parserError(peek(), "Ожидался блок кода '{...}' после for");
        return nullptr;
    }
    return parseBlock();
}

// break;
std::unique_ptr<ASTNode> Parser::parseBreak() {
    consume();          // "break"
    Close_block();      // ';'
    return std::make_unique<BreakNode>();
}

// continue;
std::unique_ptr<ASTNode> Parser::parseContinue() {
    consume();          // "continue"
    Close_block();      // ';'
    return std::make_unique<ContinueNode>();
}

// if (cond) { ... } [else ...]
std::unique_ptr<ASTNode> Parser::parseIf() {
    debugging();
    consume(); // "if"

    if (peek().type != TokenType::LParen) {
        parserError(peek(), "Ожидалась открывающая скобка '(' после 'if'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
        return nullptr;
    }
    consume(); // ')'

    if (peek().type != TokenType::LBrace) {
        parserError(peek(), "Ожидался блок кода '{...}' после условия");
        return nullptr;
    }
    auto thenBody = parseBlock();

    std::unique_ptr<ASTNode> elseBody = nullptr;
    if (check(TokenType::KwElse)) {
        elseBody = parseElse();
    }
    return std::make_unique<IfNode>(std::move(condition),
                                    std::move(thenBody),
                                    std::move(elseBody));
}

// else { ... }  /  else if (...) { ... }
std::unique_ptr<ASTNode> Parser::parseElse() {
    debugging();
    consume(); // "else"

    if (check(TokenType::KwIf)) {
        return parseIf(); // else if
    } else if (peek().type == TokenType::LBrace) {
        return parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после 'else'");
        return nullptr;
    }
}

// print(...);  /  print.ln(...);
std::unique_ptr<ASTNode> Parser::parsePrint() {
    debugging();
    consume(); // "print"

    bool newline = false;

    // Опциональный модификатор .ln
    if (check(TokenType::Dot)) {
        consume(); // '.'
        if (check(TokenType::Identifier, "ln")) {
            consume(); // "ln"
            newline = true;
        } else {
            parserError(peek(), "Ожидался модификатор 'ln' после точки в print");
        }
    }

    if (peek().type != TokenType::LParen) {
        parserError(peek(), "Ожидалась открывающая скобка '(' после print");
    }
    consume(); // '('

    auto expr = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после аргументов print");
    }
    consume(); // ')'
    Close_block();
    return std::make_unique<PrintNode>(std::move(expr), newline);
}

// Присваивание:  Identificator (=|+=|-=|*=|/=) parseLogicalOr
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    Token varName = consume();           // идентификатор
    std::string op = consume().value;    // "=", "+=", ...
    auto expr = parseLogicalOr();
    Close_block();
    return std::make_unique<AssignmentNode>(varName.value, op, std::move(expr));
}

// ============================================================
// Выражения (возвращают ExpressionNode)
// ============================================================
std::unique_ptr<ExpressionNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (peek().type == TokenType::Or) {
        std::string op = consume().value;
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (peek().type == TokenType::And) {
        std::string op = consume().value;
        auto right = parseEquality();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseEquality() {
    auto left = parseRelational();
    while (peek().type == TokenType::Equal || peek().type == TokenType::NotEqual) {
        std::string op = consume().value;
        auto right = parseRelational();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseRelational() {
    auto left = parseAdditive();
    while (peek().type == TokenType::Less      ||
           peek().type == TokenType::Greater   ||
           peek().type == TokenType::LessEq    ||
           peek().type == TokenType::GreaterEq) {
        std::string op = consume().value;
        auto right = parseAdditive();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        std::string op = consume().value;
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (peek().type == TokenType::Multiply || peek().type == TokenType::Divide) {
        std::string op = consume().value;
        auto right = parseUnary();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseUnary() {
    // !expr   /   not expr   /   -expr
    if (peek().type == TokenType::NotOp ||
        peek().type == TokenType::KwNot ||
        peek().type == TokenType::Minus) {

        Token opTok = consume();
        // нормализуем "not" -> "!" для единообразного AST
        std::string op = (opTok.type == TokenType::KwNot) ? "!" : opTok.value;
        auto operand = parseUnary(); // унарные операции могут быть вложенными
        return std::make_unique<UnaryOpNode>(op, std::move(operand));
    }
    return parsePrimary();
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    Token tok = peek();

    if (tok.type == TokenType::Int) {
        consume();
        return std::make_unique<LiteralNode>(LiteralNode::Int, tok.value);
    } else if (tok.type == TokenType::Float) {
        consume();
        return std::make_unique<LiteralNode>(LiteralNode::Float, tok.value);
    } else if (tok.type == TokenType::String) {
        consume();
        return std::make_unique<LiteralNode>(LiteralNode::String, tok.value);
    } else if (tok.type == TokenType::Bool) {
        consume();
        return std::make_unique<LiteralNode>(LiteralNode::Bool, tok.value);
    } else if (tok.type == TokenType::Identifier) {
        consume();
        auto var = std::make_unique<VariableNode>(tok.value);
        if (peek().type == TokenType::Increment || peek().type == TokenType::Decrement) {
            std::string op = consume().value;
            return std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
        }
        return var;
    } else if (tok.type == TokenType::LParen) {
        consume(); // '('
        auto expr = parseLogicalOr();
        if (peek().type != TokenType::RParen) {
            parserError(peek(), "Ожидалась закрывающая скобка ')'");
        }
        consume(); // ')'
        return expr;
    } else {
        parserError(tok, "Синтаксическая ошибка: неожиданный токен '" + tok.value + "'");
        return nullptr;
    }
}