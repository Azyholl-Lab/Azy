#include "parser.h"
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
    return {TokenType::Unknown, "", -1, -1};
}

Token Parser::peekNext() {
    if (currentPos + 1 < tokens.size()) {
        return tokens[currentPos + 1];
    }
    return {TokenType::Unknown, "", -1, -1};
}

Token Parser::consume() {
    Token tok = peek();
    if (currentPos < tokens.size()) {
        currentPos++;
    }
    return tok;
}

bool Parser::check(TokenType type, const std::string& value) {
    return (peek().type == type && peek().value == value);
}

bool Parser::checkNext(TokenType type, const std::string& value) {
    return (peekNext().type == type && peekNext().value == value);
}

void Parser::Close_block(){
    if (check(TokenType::Operator, ";")){
        consume();
    } else {
        parserError(peek(), "Синтаксическая ошибка: ожидался терминатор: '" + peek().value + "'");
    }
}

// ============================================================
// Главный цикл разбора – возвращает корневой блок
// ============================================================
std::unique_ptr<ASTNode> Parser::parse() {
    auto block = std::make_unique<BlockNode>();
    while (currentPos < tokens.size()) {
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        } else {
            // Если оператор не распознан – ошибка, но parseStatement уже вызовет parserError
            break;
        }
    }
    return block;
}

// ============================================================
// Разбор операторов (возвращают узел AST)
// ============================================================
std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (check(TokenType::Identifier, "define") || check(TokenType::Identifier, "fn")) {
        return parseFunctionDef();
    } else if (check(TokenType::Identifier, "if")) {
        return parseIf();
    } else if (check(TokenType::Identifier, "do")) {
        return parseDoWhile();
    } else if (check(TokenType::Identifier, "while")) {
        return parseWhile();
    } else if (check(TokenType::Identifier, "for")) {
        return parseFor();
    } else if (check(TokenType::Identifier, "struct")) {
        return parseStructDef(false);
    } else if (check(TokenType::Identifier, "class")) {
        return parseStructDef(true);
    } else if (peek().type == TokenType::Identifier && checkNext(TokenType::Operator, "=")) {
        return parseAssignment();
    } else {
        parserError(peek(), "Синтаксическая ошибка: неожиданный токен '" + peek().value + "'");
        consume(); 
        return nullptr;
    }
}
std::unique_ptr<ASTNode> Parser::parseBlock() {
    debugging();
    consume(); // '{'

    auto block = std::make_unique<BlockNode>();
    while (peek().value != "}" && peek().type != TokenType::Unknown) {
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        }
    }

    if (peek().value != "}") {
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
    Token token = consume(); // "do"

    std::unique_ptr<ASTNode> body;
    if (peek().value == "{") {
        body = parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после 'do'");
        return nullptr;
    }

    if (check(TokenType::Identifier, "while")) {
        token = consume(); // "while"
        if (peek().value != "(") {
            parserError(peek(), "Ожидалась открывающая скобка '(' после 'while'");
            return nullptr;
        }
        consume(); // '('
        auto condition = parseLogicalOr();
        if (peek().value != ")") {
            parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
            return nullptr;
        }
        consume(); // ')'
        return std::make_unique<DoWhileNode>(std::move(body), std::move(condition));
    } else {
        parserError(peek(), "Ожидался идентификатор 'while' после блока кода");
        return nullptr;
    }
}

// Узел while (BinaryOpNode) {}
std::unique_ptr<ASTNode> Parser::parseWhile() {
    debugging();
    Token token = consume(); // "while"

    if (peek().value != "(") {
        parserError(peek(), "Ожидалась открывающая скобка '(' после 'while'");
        return nullptr;
    }
    consume(); // '('
    auto condition = parseLogicalOr();
    if (peek().value != ")") {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
        return nullptr;
    }
    consume(); // ')'

    std::unique_ptr<ASTNode> body;
    if (peek().value == "{") {
        body = parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после условия");
        return nullptr;
    }
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

// Узел If (BinaryOpNode) {Block_code}
std::unique_ptr<ASTNode> Parser::parseIf() {
    debugging();
    Token token = consume(); // "if"

    if (peek().value != "(") {
        parserError(peek(), "Ожидалась открывающая скобка '(' после 'if'");
        return nullptr;
    }
    consume(); // '('
    auto condition = parseLogicalOr();
    if (peek().value != ")") {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия");
        return nullptr;
    }
    consume(); // ')'

    std::unique_ptr<ASTNode> thenBody;
    if (peek().value == "{") {
        thenBody = parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после условия");
        return nullptr;
    }

    std::unique_ptr<ASTNode> elseBody = nullptr;
    if (check(TokenType::Identifier, "else")) {
        elseBody = parseElse();
    }
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBody), std::move(elseBody));
}

// Узел else {Block_code}
std::unique_ptr<ASTNode> Parser::parseElse() {
    debugging();
    consume(); // "else"

    if (check(TokenType::Identifier, "if")) {
        return parseIf(); // else if
    } else if (peek().value == "{") {
        return parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после 'else'");
        return nullptr;
    }
}

// Узел вывода текста
std::unique_ptr<ASTNode> Parser::parsePrint() {
    debugging();
    consume();

    bool newline = false;

    // Проверяем опциональный модификатор .ln
    if (check(TokenType::Operator, ".")) {
        consume(); // съедаем точку
        if (check(TokenType::Identifier, "ln")) {
            consume(); // съедаем "ln"
            newline = true;
        } else {
            parserError(peek(), "Ожидался модификатор 'ln' после точки в print");
        }
    }
    if (peek().value != "(") {
        parserError(peek(), "Ожидалась открывающая скобка '(' после print");
    }
    consume(); // '('

    auto expr = parseLogicalOr();
    if (peek().value != ")") {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после аргументов print");
    }
    consume(); // ')'
    Close_block();
    return std::make_unique<PrintNode>(std::move(expr), newline);
}

// Узел присваивания 
// Identificator = parseLogicalOr
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    Token varName = consume(); // идентификатор
    consume(); // '='
    auto expr = parseLogicalOr();
    Close_block();
    return std::make_unique<AssignmentNode>(varName.value, std::move(expr));
}

// ============================================================
// Выражения (возвращают ExpressionNode)
// ============================================================
std::unique_ptr<ExpressionNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (true) {
        if (peek().value == "||") {
            consume();
            auto right = parseLogicalAnd();
            left = std::make_unique<BinaryOpNode>("||", std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (true) {
        if (peek().value == "&&") {
            consume();
            auto right = parseEquality();
            left = std::make_unique<BinaryOpNode>("&&", std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseEquality() {
    auto left = parseRelational();
    while (true) {
        if (peek().value == "==" || peek().value == "!=") {
            std::string op = peek().value;
            consume();
            auto right = parseRelational();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseRelational() {
    auto left = parseAdditive();
    while (true) {
        if (peek().value == "<" || peek().value == ">" ||
            peek().value == "<=" || peek().value == ">=") {
            std::string op = peek().value;
            consume();
            auto right = parseAdditive();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (true) {
        if (peek().value == "+" || peek().value == "-") {
            std::string op = peek().value;
            consume();
            auto right = parseMultiplicative();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (true) {
        if (peek().value == "*" || peek().value == "/") {
            std::string op = peek().value;
            consume();
            auto right = parseUnary();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseUnary() {
    if (peek().value == "!" || peek().value == "not" || peek().value == "-") {
        std::string op = peek().value;
        consume();
        auto operand = parseUnary(); // унарные операции могут быть вложенными
        return std::make_unique<UnaryOpNode>(op, std::move(operand));
    } else {
        return parsePrimary();
    }
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
    } else if (tok.type == TokenType::Identifier) {
        consume();
        return std::make_unique<VariableNode>(tok.value);
    } else if (tok.value == "(") {
        consume(); // '('
        auto expr = parseLogicalOr();
        if (peek().value != ")") {
            parserError(peek(), "Ожидалась закрывающая скобка ')'");
        }
        consume(); // ')'
        return expr;
    } else {
        parserError(tok, "Синтаксическая ошибка: неожиданный токен '" + tok.value + "'");
        return nullptr;
    }
}

std::unique_ptr<ASTNode> Parser::parseFor() {
    consume(); // "for"
    if (peek().value != "(") parserError(peek(), "Ожидалась '(' после 'for'");
    consume(); // "("

    std::unique_ptr<ASTNode> init = nullptr;
    if (peek().value != ";") {
        // Для прототипа поддерживаем только присваивание в инициализации (e.g., i = 0)
        if (peek().type == TokenType::Identifier && checkNext(TokenType::Operator, "=")) {
            init = parseAssignment();
        } else {
            parserError(peek(), "В инициализации for ожидается присваивание (e.g., i = 0)");
        }
    }
    if (peek().value != ";") parserError(peek(), "Ожидалась ';' после инициализации в for");
    consume(); // ";"

    std::unique_ptr<ExpressionNode> condition = nullptr;
    if (peek().value != ";") {
        condition = parseLogicalOr();
    }
    if (peek().value != ";") parserError(peek(), "Ожидалась ';' после условия в for");
    consume(); // ";"

    std::unique_ptr<ASTNode> step = nullptr;
    if (peek().value != ")") {
        if (peek().type == TokenType::Identifier && checkNext(TokenType::Operator, "=")) {
            step = parseAssignment();
        } else {
            parserError(peek(), "В шаге for ожидается присваивание (e.g., i = i + 1)");
        }
    }
    if (peek().value != ")") parserError(peek(), "Ожидалась ')' после шага в for");
    consume(); // ")"

    std::unique_ptr<ASTNode> body;
    if (peek().value == "{") {
        body = parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после for");
    }

    return std::make_unique<ForNode>(std::move(init), std::move(condition), std::move(step), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseStructDef(bool isClass) {
    Token keywordToken = consume(); // "struct" или "class"
    Token nameToken = consume();
    if (nameToken.type != TokenType::Identifier) {
        parserError(nameToken, "Ожидалось имя после '" + keywordToken.value + "'");
    }

    if (peek().value != "{") parserError(peek(), "Ожидалась '{' после имени struct/class");
    consume(); // "{"

    std::vector<std::unique_ptr<FieldDeclNode>> fields;
    while (peek().value != "}" && peek().type != TokenType::Unknown) {
        // Ожидаем: Тип Имя ;
        Token typeToken = consume();
        Token fieldToken = consume();
        
        if (peek().value != ";") {
            parserError(peek(), "Ожидалась ';' после объявления поля");
        }
        consume(); // ";"

        fields.push_back(std::make_unique<FieldDeclNode>(typeToken.value, fieldToken.value));
    }

    if (peek().value != "}") parserError(peek(), "Незакрытая '{' в определении struct/class");
    consume(); // "}"

    return std::make_unique<StructDefNode>(nameToken.value, isClass, std::move(fields));
}
std::unique_ptr<ASTNode> Parser::parseFunctionDef() {
    Token keywordTok = consume(); // "define" или "fn"
    
    Token nameTok = consume();
    if (nameTok.type != TokenType::Identifier) {
        parserError(nameTok, "Ожидалось имя функции после '" + keywordTok.value + "'");
    }

    if (peek().value != "(") {
        parserError(peek(), "Ожидалась '(' после имени функции");
    }
    consume(); // "("

    std::vector<ParamNode> params;
    // Парсим параметры, пока не встретим ')'
    while (peek().value != ")" && peek().type != TokenType::Unknown) {
        Token typeTok = consume();
        Token nameParamTok = consume();
        
        params.push_back({typeTok.value, nameParamTok.value});

        if (peek().value == ",") {
            consume(); // запятая между параметрами
        } else if (peek().value != ")") {
            parserError(peek(), "Ожидалась ',' или ')' в списке параметров");
        }
    }

    if (peek().value != ")") {
        parserError(peek(), "Незакрытая '(' в объявлении функции");
    }
    consume(); // ")"

    // Опционально: здесь можно добавить парсинг возвращаемого типа (например, "-> int")
    // Для прототипа оставим "void"
    std::string returnType = "void";
    if (check(TokenType::Operator, "->")) {
        consume(); // "->"
        returnType = consume().value; // читаем тип возврата
    }

    std::unique_ptr<ASTNode> body = nullptr;
    if (peek().value == "{") {
        body = parseBlock(); // parseBlock уже возвращает unique_ptr<ASTNode> (внутри которого BlockNode)
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' для тела функции");
    }

    return std::make_unique<FunctionDefNode>(nameTok.value, std::move(params), returnType, std::move(body));
}

// ============================================================
// 
// ============================================================