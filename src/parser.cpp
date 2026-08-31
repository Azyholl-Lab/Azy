#include "parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), currentPos(0) {}

Token Parser::peek() {
    if (currentPos < tokens.size()) {
        return tokens[currentPos];
    }
    return {TokenType::Unknown, "", -1, -1}; // Конец файла/токенов
}

Token Parser::consume() {
    Token tok = peek();
    if (currentPos < tokens.size()) {
        currentPos++;
    }
    return tok;
}

bool Parser::check(TokenType type, const std::string& value) {
    Token tok = peek();
    return (tok.type == type && tok.value == value);
}

// Главный цикл разбора программы
void Parser::parse() {
    while (currentPos < tokens.size()) {
        parseStatement();
    }
}

void Parser::parserError(const Token& token, const std::string& message) {
    std::string errorMsg = "[Ошибка парсера] " + message + " (строка: " + std::to_string(token.line) + ", символ: " + std::to_string(token.column) + ", токен: '" + token.value + "')";
    throw std::runtime_error(errorMsg);
}

// ==========================================
// ГЛАВНОЕ ВЕТВЛЕНИЕ
// ==========================================
void Parser::parseStatement() {
    Token tok = peek();

    // 1. Ветвление для условного оператора: if
    if (tok.type == TokenType::Identifier && tok.value == "if") {
        parseIf();
    }
    else if (tok.value == "{") {
        parseBlock();
    }
    // // 2. Ветвление для цикла while
    // else if (tok.type == TokenType::Identifier && tok.value == "while") {
    //     parseWhile();
    // }
    // // 3. Ветвление для цикла for
    // else if (tok.type == TokenType::Identifier && tok.value == "for") {
    //     parseFor();
    // }
    // // 4. Ветвление для объявления функции: func
    // else if (tok.type == TokenType::Identifier && tok.value == "func") {
    //     parseFunc();
    // }
    // 5. По умолчанию: математические действия, присваивания или вызовы (начинающиеся с переменной или числа)
    // else {
    //     parseExpression();
    // }
}

// --- Реализация конкретных разборщиков ---
// if(true){ parseStatement }
void Parser::parseIf() {
    consume(); // Съедаем токен "if"
    std::cout << "[Парсер] Обнаружено ветвление IF\n";

    // 1. Проверяем и съедаем открывающую скобку условия
    if (peek().value != "(") {
         parserError(peek(), "Ожидалась открывающая скобка '(' после 'if'");
    }
    consume(); // Съедаем '('

    parseExpression(); 

    // 3. Проверяем и съедаем закрывающую скобку
    if (peek().value != ")") {
        parserError(peek(), "Ожидалась закрывающая скобка ')' после условия if");
    }
    consume(); // Съедаем ')'

    // 4. Разбираем тело блока кода
    if (peek().value == "{"){
        parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после условия if");
    }

    // 5. проверка наличия ветки else
    if (check(TokenType::Identifier, "else")) {
        consume(); // Съедаем "else"
        std::cout << "[Парсер] Обнаружена ветка ELSE\n";
        parseElse(); // Разбираем тело else
    }
}

void Parser::parseElse(){
    std::cout << "[Парсер] Обнаружено ветвление ELSE\n";

    if (peek().type == TokenType::Identifier && peek().value == "if") {
        parseIf();
    } else if (peek().value == "{"){
        parseBlock();
    } else {
        parserError(peek(), "Ожидался блок кода '{...}' после условия else");
    }
}


void Parser::parseBlock() {
    consume(); // Съедаем '{'
    std::cout << "[Парсер] Начало блока кода\n";

    while (peek().value != "}" && peek().type != TokenType::Unknown) {
        parseStatement();
    }

    if (peek().value != "}") {
         parserError(peek(), "Ожидалась закрывающая фигурная скобка '}'");
    }
    consume(); // Съедаем '}'
    std::cout << "[Парсер] Конец блока кода\n";
}



void Parser::parseExpression() {
    // 1. Разбираем левую часть (операнд или фактор)
    parseFactor();

    // 2. Проверяем наличие операторов (сравнения или арифметики)
    Token tok = peek();
    while (tok.value == "==" || tok.value == "!=" || tok.value == "<" || 
           tok.value == ">" || tok.value == "&&" || tok.value == "||" || 
           tok.value == "+" || tok.value == "-") {
        consume(); // Съедаем оператор
        parseFactor(); // Разбираем правую часть выражения
        tok = peek();
    }
}

void Parser::parseFactor() {
    Token tok = peek();

    if (tok.type == TokenType::Int || tok.type == TokenType::Float || tok.type == TokenType::Identifier) {
        consume(); // Забираем число, переменную или ключевое слово (true/false)
    } 
    else if (tok.value == "(") {
        consume(); // Съедаем открывающую скобку
        parseExpression(); // Важно: внутри скобок разбираем выражение, а не statement
        
        Token closing = consume();
        if (closing.value != ")") {
             parserError(peek(), "Ожидалась закрывающая скобка");
        }
    } 
    else {
         parserError(peek(), "Синтаксическая ошибка: неожиданный токен '" + tok.value + "'");
    }
}


