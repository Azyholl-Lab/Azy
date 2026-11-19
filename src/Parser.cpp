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
        parserError(ErrCode::P001_ExpectedSemicolon, peek(),
                    "(получен '" + peek().value + "')");
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
    // --- Объявления ---
    if (check(TokenType::KwVar)) {
        return parseVarDecl();
    } else if (check(TokenType::KwFn)) {
        return parseFunction();
    } else if (check(TokenType::KwReturn)) {
        return parseReturn();

    // --- Управляющие конструкции ---
    } else if (check(TokenType::KwIf)) {
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

    // --- Встроенные функции-операторы ---
    } else if (check(TokenType::KwPrint)) {
        return parsePrint();
    } else if (check(TokenType::KwInput)) {
        return parseInput();

    // --- Присваивание: id (=|+=|-=|*=|/=) expr ; ---
    } else if (peek().type == TokenType::Identifier &&
            (checkNext(TokenType::Assign)      ||
             checkNext(TokenType::PlusAssign)  ||
             checkNext(TokenType::MinusAssign) ||
             checkNext(TokenType::MultAssign)  ||
             checkNext(TokenType::DivAssign))) {
        return parseAssignment();

    // --- Постфикс: id++ ; / id-- ; ---
    } else if (peek().type == TokenType::Identifier &&
            (checkNext(TokenType::Increment) ||
             checkNext(TokenType::Decrement))) {
        return parsePostfixStatement();

    // --- Пустой оператор ';' ---
    } else if (check(TokenType::Semicolon)) {
        consume();
        return nullptr;   // no-op
    }

    // --- Всё остальное — ошибка ---
    parserError(ErrCode::P009_UnexpectedToken, peek());
    return nullptr; // unreachable
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
        parserError(ErrCode::P005_ExpectedClosingBrace, peek());
        return nullptr;
    }
    debugging();
    consume(); // '}'
    return block;
}

// ============================================================
// Объявления: var / fn / def / return
// ============================================================

// var x;  |  var x = 5;  |  var x = 5, y = 10;
std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    debugging();
    consume(); // "var"

    auto node = std::make_unique<VarDeclNode>();

    while (true) {
        if (peek().type != TokenType::Identifier) {
            parserError(ErrCode::P006_ExpectedIdentifier, peek(), "после 'var'");
            return nullptr;
        }
        SourcePos namePos(peek());       // ⭐
        std::string name = consume().value;

        std::unique_ptr<ExpressionNode> init = nullptr;
        if (check(TokenType::Assign)) {
            consume();
            init = parseLogicalOr();
            if (!init) {
                parserError(ErrCode::P007_ExpectedExpression, peek(),
                            "после '=' в объявлении '" + name + "'");
                return nullptr;
            }
        }

        node->decls.emplace_back(std::move(name), std::move(init), namePos); // ⭐

        if (check(TokenType::Comma)) { consume(); continue; }
        break;
    }

    Close_block();
    return node;
}

// fn name(a, b) { ... }  |  def name() { ... }
std::unique_ptr<ASTNode> Parser::parseFunction() {
    debugging();
    consume(); // "fn" или "def"

    if (peek().type != TokenType::Identifier) {
        parserError(ErrCode::P006_ExpectedIdentifier, peek(), "имени функции");
        return nullptr;
    }
    SourcePos namePos(peek());
    std::string name = consume().value;

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после имени функции '" + name + "'");
        return nullptr;
    }
    consume();

    std::vector<std::string> params;
    if (peek().type != TokenType::RParen) {
        if (peek().type != TokenType::Identifier) {
            parserError(ErrCode::P006_ExpectedIdentifier, peek(),
                        "параметра функции");
            return nullptr;
        }
        params.push_back(consume().value);
        while (peek().type == TokenType::Comma) {
            consume();
            if (peek().type != TokenType::Identifier) {
                parserError(ErrCode::P006_ExpectedIdentifier, peek(),
                            "параметра после ','");
                return nullptr;
            }
            params.push_back(consume().value);
        }
    }

    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после параметров функции");
        return nullptr;
    }
    consume();

    if (peek().type != TokenType::LBrace) {
        parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                    "тела функции '" + name + "'");
        return nullptr;
    }
    auto body = parseBlock();

    return std::make_unique<FunctionNode>(
        std::move(name), std::move(params), std::move(body), namePos);
}

// return;  |  return expr;
std::unique_ptr<ASTNode> Parser::parseReturn() {
    debugging();
    SourcePos pos(peek());
    consume(); // "return"

    std::unique_ptr<ExpressionNode> expr = nullptr;
    if (peek().type != TokenType::Semicolon) {
        expr = parseLogicalOr();
        if (!expr) {
            parserError(ErrCode::P007_ExpectedExpression, peek(), "после 'return'");
            return nullptr;
        }
    }
    Close_block();
    return std::make_unique<ReturnNode>(std::move(expr), pos);
}

// ============================================================
// Конкретные операторы
// ============================================================
std::unique_ptr<ASTNode> Parser::parseDoWhile() {
    debugging();
    consume(); // "do"

    if (peek().type != TokenType::LBrace) {
        parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                    "после 'do'");
        return nullptr;
    }
    auto body = parseBlock();

    if (!check(TokenType::KwWhile)) {
        parserError(ErrCode::P104_MissingWhileAfterDo, peek());
        return nullptr;
    }
    consume(); // "while"

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после 'while'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после условия");
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
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после 'while'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после условия");
        return nullptr;
    }
    consume(); // ')'

    if (peek().type != TokenType::LBrace) {
        parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                    "после условия");
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
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(), "после 'for'");
        return nullptr;
    }
    consume();

    if (peek().type != TokenType::Identifier) {
        parserError(ErrCode::P006_ExpectedIdentifier, peek(), "переменной цикла");
        return nullptr;
    }

    std::string varName = peek().value;
    SourcePos varNamePos(peek());
    Token next = peekNext();

    // --- ФОРМА 3: for (i in ...) ---
    if (next.type == TokenType::KwIn) {
        consume(); consume();
        auto iterable = parseLogicalOr();
        if (!iterable) return nullptr;
        if (peek().type != TokenType::RParen) {
            parserError(ErrCode::P003_ExpectedClosingParen, peek(), "после for-in");
            return nullptr;
        }
        consume();
        auto body = parseForBody();
        if (!body) return nullptr;
        auto node = std::make_unique<ForNode>();
        node->varName   = varName;
        node->varNamePos = varNamePos;
        node->iterable  = std::move(iterable);
        node->isForeach = true;
        node->body      = std::move(body);
        return node;
    }

    // --- ФОРМА 1: for (i == x) ---
    if (next.type == TokenType::Equal) {
        auto cond = parseLogicalOr();
        if (!cond) return nullptr;
        if (peek().type != TokenType::RParen) {
            parserError(ErrCode::P003_ExpectedClosingParen, peek(), "после условия for");
            return nullptr;
        }
        consume();
        auto body = parseForBody();
        if (!body) return nullptr;
        auto node = std::make_unique<ForNode>();
        node->varName    = varName;
        node->varNamePos = varNamePos;
        node->condition  = std::move(cond);
        node->body       = std::move(body);
        return node;
    }

    // --- ФОРМА 2: for (i = 0; ...; ...) ---
    if (next.type == TokenType::Assign) {
        auto init = parseForInit();
        if (!init) return nullptr;
        if (peek().type != TokenType::Semicolon) {
            parserError(ErrCode::P001_ExpectedSemicolon, peek(),
                        "после инициализации for");
            return nullptr;
        }
        consume();
        auto cond = parseLogicalOr();
        if (!cond) return nullptr;
        if (peek().type != TokenType::Semicolon) {
            parserError(ErrCode::P001_ExpectedSemicolon, peek(),
                        "после условия for");
            return nullptr;
        }
        consume();
        auto step = parseForStep();
        if (!step) return nullptr;
        if (peek().type != TokenType::RParen) {
            parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                        "после шага for");
            return nullptr;
        }
        consume();
        auto body = parseForBody();
        if (!body) return nullptr;
        auto node = std::make_unique<ForNode>();
        node->varName    = varName;
        node->varNamePos = varNamePos; 
        node->init       = std::move(init);
        node->condition  = std::move(cond);
        node->step       = std::move(step);
        node->body       = std::move(body);
        return node;
    }

    parserError(ErrCode::P100_UnknownForForm, peek());
    return nullptr;
}

// Отдельный оператор: i++;  или  i--;
std::unique_ptr<ASTNode> Parser::parsePostfixStatement() {
    SourcePos namePos(peek());
    Token name = consume();
    std::string op = consume().value;
    auto var = std::make_unique<VariableNode>(name.value, namePos);
    Close_block();
    auto unary = std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
    return std::make_unique<ExpressionStatementNode>(std::move(unary));
}

// init в полной форме: i = <expr>
std::unique_ptr<ASTNode> Parser::parseForInit() {
    if (peek().type != TokenType::Identifier) {
        parserError(ErrCode::P006_ExpectedIdentifier, peek(), "в инициализации for");
        return nullptr;
    }
    SourcePos namePos(peek());
    std::string name = consume().value;

    if (!check(TokenType::Assign)) {
        parserError(ErrCode::P010_ExpectedAssign, peek(), "в инициализации for");
        return nullptr;
    }
    consume();
    auto expr = parseLogicalOr();
    return std::make_unique<AssignmentNode>(name, "=", std::move(expr), namePos);
}

// step в полной форме: i++ / i-- / i = expr
std::unique_ptr<ASTNode> Parser::parseForStep() {
    if (peek().type != TokenType::Identifier) {
        parserError(ErrCode::P006_ExpectedIdentifier, peek(), "в шаге for");
        return nullptr;
    }
    SourcePos namePos(peek());
    std::string name = consume().value;

    if (peek().type == TokenType::Increment || peek().type == TokenType::Decrement) {
        std::string op = consume().value;
        auto var = std::make_unique<VariableNode>(name, namePos);
        return std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
    }

    if (check(TokenType::Assign)) {
        consume();
        auto expr = parseLogicalOr();
        return std::make_unique<AssignmentNode>(name, "=", std::move(expr), namePos);
    }

    parserError(ErrCode::P011_ExpectedForStep, peek());
    return nullptr;
}

// тело цикла — блок { ... }
std::unique_ptr<ASTNode> Parser::parseForBody() {
    if (peek().type != TokenType::LBrace) {
        parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                    "после for");
        return nullptr;
    }
    return parseBlock();
}

// break;
std::unique_ptr<ASTNode> Parser::parseBreak() {
    SourcePos pos(peek());
    consume();
    Close_block();
    return std::make_unique<BreakNode>(pos);
}

// continue;
std::unique_ptr<ASTNode> Parser::parseContinue() {
    SourcePos pos(peek());
    consume();
    Close_block();
    return std::make_unique<ContinueNode>(pos);
}

// if (cond) { ... } [else ...]
std::unique_ptr<ASTNode> Parser::parseIf() {
    debugging();
    consume(); // "if"

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после 'if'");
        return nullptr;
    }
    consume(); // '('

    auto condition = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после условия");
        return nullptr;
    }
    consume(); // ')'

    if (peek().type != TokenType::LBrace) {
        parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                    "после условия");
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
    }

    parserError(ErrCode::P004_ExpectedOpeningBrace, peek(),
                "после 'else'");
    return nullptr;
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
            parserError(ErrCode::P008_ExpectedModifier, peek(),
                        "в print — ожидался 'ln'");
            return nullptr;
        }
    }

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после print");
        return nullptr;
    }
    consume(); // '('

    auto expr = parseLogicalOr();
    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после аргументов print");
        return nullptr;
    }
    consume(); // ')'
    Close_block();
    return std::make_unique<PrintNode>(std::move(expr), newline);
}

// input(x);  input(x, y);  input(x, y, z);
// input.line(x);  input.line(x, y);
std::unique_ptr<ASTNode> Parser::parseInput() {
    debugging();
    consume(); // "input"

    bool readLine = false;
    if (check(TokenType::Dot)) {
        consume(); // '.'
        if (!check(TokenType::Identifier, "line")) {
            parserError(ErrCode::P008_ExpectedModifier, peek(),
                        "в input — ожидался 'line'");
            return nullptr;
        }
        consume(); // "line"
        readLine = true;
    }

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после input");
        return nullptr;
    }
    consume(); // '('

    // input() без аргументов — это выражение, а не оператор
    if (peek().type == TokenType::RParen) {
        parserError(ErrCode::P101_InputEmpty, peek());
        return nullptr;
    }

    std::vector<std::string> vars;

    if (peek().type != TokenType::Identifier) {
        parserError(ErrCode::P006_ExpectedIdentifier, peek(),
                    "переменной в input");
        return nullptr;
    }
    vars.push_back(consume().value);

    while (peek().type == TokenType::Comma) {
        consume(); // ','
        if (peek().type != TokenType::Identifier) {
            parserError(ErrCode::P006_ExpectedIdentifier, peek(),
                        "переменной после ',' в input");
            return nullptr;
        }
        vars.push_back(consume().value);
    }

    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после аргументов input");
        return nullptr;
    }
    consume(); // ')'
    Close_block(); // ';'

    return std::make_unique<InputNode>(std::move(vars), readLine);
}

// input()       — читает слово, возвращает строку
// input.line()  — читает строку целиком, возвращает строку
std::unique_ptr<ExpressionNode> Parser::parseInputExpression() {
    debugging();
    consume(); // "input"

    bool readLine = false;
    if (check(TokenType::Dot)) {
        consume(); // '.'
        if (!check(TokenType::Identifier, "line")) {
            parserError(ErrCode::P008_ExpectedModifier, peek(),
                        "в input — ожидался 'line'");
            return nullptr;
        }
        consume(); // "line"
        readLine = true;
    }

    if (peek().type != TokenType::LParen) {
        parserError(ErrCode::P002_ExpectedOpeningParen, peek(),
                    "после input");
        return nullptr;
    }
    consume(); // '('

    // input(y) как выражение — не поддерживаем
    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P102_InputWithArgsAsExpr, peek());
        return nullptr;
    }
    consume(); // ')'

    return std::make_unique<InputExprNode>(readLine);
}

// Присваивание:  Identificator (=|+=|-=|*=|/=) parseLogicalOr ;
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    SourcePos namePos(peek());
    Token varName = consume();
    std::string op = consume().value;
    auto expr = parseLogicalOr();
    Close_block();
    return std::make_unique<AssignmentNode>(varName.value, op, std::move(expr), namePos);
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

// Вызов функции: name(arg1, arg2, ...)
std::unique_ptr<ExpressionNode> Parser::parseCall() {
    Token calleeTok = peek();            // Identifier
    consume(); // имя функции
    consume(); // '('

    auto node = std::make_unique<CallNode>(calleeTok.value, SourcePos(calleeTok));

    if (peek().type != TokenType::RParen) {
        auto arg = parseLogicalOr();
        if (!arg) {
            parserError(ErrCode::P007_ExpectedExpression, peek(),
                        "в вызове функции '" + calleeTok.value + "'");
            return nullptr;
        }
        node->args.push_back(std::move(arg));

        while (peek().type == TokenType::Comma) {
            consume();
            auto nextArg = parseLogicalOr();
            if (!nextArg) {
                parserError(ErrCode::P007_ExpectedExpression, peek(),
                            "в вызове функции после ','");
                return nullptr;
            }
            node->args.push_back(std::move(nextArg));
        }
    }

    if (peek().type != TokenType::RParen) {
        parserError(ErrCode::P003_ExpectedClosingParen, peek(),
                    "после аргументов вызова '" + calleeTok.value + "'");
        return nullptr;
    }
    consume(); // ')'
    return node;
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
    } else if (tok.type == TokenType::KwInput) {
        return parseInputExpression();
    } else if (tok.type == TokenType::Identifier) {
        if (peekNext().type == TokenType::LParen) {
            return parseCall();
        }

        consume();
        auto var = std::make_unique<VariableNode>(tok.value, SourcePos(tok));
        if (peek().type == TokenType::Increment || peek().type == TokenType::Decrement) {
            std::string op = consume().value;
            return std::make_unique<UnaryOpNode>(op + "_post", std::move(var));
        }
        return var;
    } else if (tok.type == TokenType::LParen) {
        consume();
        auto expr = parseLogicalOr();
        if (peek().type != TokenType::RParen) {
            parserError(ErrCode::P003_ExpectedClosingParen, peek());
            return nullptr;
        }
        consume();
        return expr;
    }

    parserError(ErrCode::P009_UnexpectedToken, tok);
    return nullptr;
}