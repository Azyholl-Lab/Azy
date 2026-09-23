#pragma once
#include <string>
#include <ostream>

// Категории токенов для языка Azy
enum class TokenType {
    // Служебные
    EndOfFile,
    Unknown,

    // Идентификаторы и литералы
    Identifier, // Идентификатор
    Int,        // числовое значение
    Float,      // числовое значение с плавающей точкой
    String,     // строка
    Bool,       // true, false

    // Ключевые слова (управляющие конструкции, переменные и т.д.)
    KwLet,      // let (или аналог объявления)
    KwFn,       // def / fn
    KwIf,       // if
    KwElse,     // else
    KwWhile,    // while
    KwDo,       // do
    KwFor,      // for
    KwIn,       // in (для foreach)
    KwReturn,   // return
    KwBreak,    // break
    KwContinue, // continue
    KwNot,      // условный оператор/слово not

    // Арифметические операторы
    Plus,       // +
    Minus,      // -
    Multiply,   // *
    Divide,     // /

    // Операторы присваивания
    Assign,     // =
    PlusAssign, // +=
    MinusAssign,// -=
    MultAssign, // *=
    DivAssign,  // /=

    // Инкремент / декремент
    Increment,  // ++
    Decrement,  // --

    // Операторы сравнения
    Equal,      // ==
    NotEqual,   // !=
    Less,       // <
    Greater,    // >
    LessEq,     // <=
    GreaterEq,  // >=

    // Логические операторы
    And,        // &&
    Or,         // ||
    NotOp,      // !

    // Разделители и знаки препинания
    LParen,     // (
    RParen,     // )
    LBrace,     // {
    RBrace,     // }
    LBracket,   // [
    RBracket,   // ]
    Semicolon,  // ;
    Comma,      // ,
    Colon,      // :
    Dot         // . 
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

// Удобный вывод типа токена в поток (для отладки и печати)
inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    switch (type) {
        case TokenType::EndOfFile:  os << "EndOfFile"; break;
        case TokenType::Unknown:     os << "Unknown"; break;
        case TokenType::Identifier: os << "Identifier"; break;
        case TokenType::Int:        os << "Int"; break;
        case TokenType::Float:      os << "Float"; break;
        case TokenType::String:     os << "String"; break;
        case TokenType::Bool:       os << "Bool"; break;

        case TokenType::KwLet:      os << "KwLet"; break;
        case TokenType::KwFn:       os << "KwFn"; break;
        case TokenType::KwIf:       os << "KwIf"; break;
        case TokenType::KwElse:     os << "KwElse"; break;
        case TokenType::KwWhile:    os << "KwWhile"; break;
        case TokenType::KwDo:       os << "KwDo"; break;
        case TokenType::KwFor:      os << "KwFor"; break;
        case TokenType::KwIn:       os << "KwIn"; break;
        case TokenType::KwReturn:   os << "KwReturn"; break;
        case TokenType::KwBreak:    os << "KwBreak"; break;
        case TokenType::KwContinue: os << "KwContinue"; break;
        case TokenType::KwNot:      os << "KwNot"; break;

        case TokenType::Plus:       os << "Plus"; break;
        case TokenType::Minus:      os << "Minus"; break;
        case TokenType::Multiply:   os << "Multiply"; break;
        case TokenType::Divide:     os << "Divide"; break;

        case TokenType::Assign:     os << "Assign"; break;
        case TokenType::PlusAssign: os << "PlusAssign"; break;
        case TokenType::MinusAssign:os << "MinusAssign"; break;
        case TokenType::MultAssign: os << "MultAssign"; break;
        case TokenType::DivAssign:  os << "DivAssign"; break;

        case TokenType::Increment:  os << "Increment"; break;
        case TokenType::Decrement:  os << "Decrement"; break;

        case TokenType::Equal:      os << "Equal"; break;
        case TokenType::NotEqual:   os << "NotEqual"; break;
        case TokenType::Less:       os << "Less"; break;
        case TokenType::Greater:    os << "Greater"; break;
        case TokenType::LessEq:     os << "LessEq"; break;
        case TokenType::GreaterEq:  os << "GreaterEq"; break;

        case TokenType::And:        os << "And"; break;
        case TokenType::Or:         os << "Or"; break;
        case TokenType::NotOp:      os << "NotOp"; break;

        case TokenType::LParen:     os << "LParen"; break;
        case TokenType::RParen:     os << "RParen"; break;
        case TokenType::LBrace:     os << "LBrace"; break;
        case TokenType::RBrace:     os << "RBrace"; break;
        case TokenType::LBracket:   os << "LBracket"; break;
        case TokenType::RBracket:   os << "RBracket"; break;
        case TokenType::Semicolon:  os << "Semicolon"; break;
        case TokenType::Comma:      os << "Comma"; break;
        case TokenType::Colon:      os << "Colon"; break;
        case TokenType::Dot:        os << "Dot"; break;
    }
    return os;
}