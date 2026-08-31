#pragma once
#include <string>

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