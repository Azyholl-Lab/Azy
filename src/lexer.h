#pragma once
#include <string>
#include <cstdio>

// Категории токенов
enum class TokenType {
    Identifier, 
    Int,        
    Float,      
    Operator,   
    String,     
    Unknown     
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

// Функция, которая возвращает следующий токен из потока
Token gettok(FILE *stream);