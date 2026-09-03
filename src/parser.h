#pragma once
#include <vector>
#include <string>
#include <sstream> 
#include <stdexcept>   
#include <utility>     
#include "Token.h"
#include <iostream>

class Parser {
private:
    std::vector<Token> tokens;
    size_t currentPos = 0;

    // Вспомогательные методы для навигации по токенам
    Token peek();                                                       // Посмотреть текущий токен без сдвига
    Token peekNext();                                                   // Посмотреть на один токен вперед без сдвига
    Token consume();                                                    // Взять текущий токен и сдвинуть указатель вперед
    bool check(TokenType type, const std::string& value);               // Проверить текущий токен
    bool checkNext(TokenType type, const std::string& value);           // Проверить следующий токен

    void debugging() {
        Token token = peek();
        std::cout << "[Парсер] " << token.value << "\n";
    }
    
    struct ASTNode {
        virtual ~ASTNode() = default;
    };

public:
    Parser(const std::vector<Token>& toks);
    void parse();         // Главный метод запуска

    template <typename... Args>
    void parserError(const Token& token, Args&&... args) {
        std::ostringstream msgStream;
        (msgStream << ... << std::forward<Args>(args));

        std::string errorMsg = "[Ошибка парсера] " + msgStream.str() + 
            " (строка: " + std::to_string(token.line) + 
            ", символ: " + std::to_string(token.column) + 
            ", токен: '" + token.value + "')";
            
        throw std::runtime_error(errorMsg);
    }

    // Функции-обработчики для разных конструкций (ветвления)
    void parseStatement();      // точка входа
    
    void parseIF();
    void parseElse();

    void parseWhile();
    void parseDo_While();
    
   void parseAssignment();      // Оператор присваивания



    void parseBlock();

    void parseLogicalOr();              //Логическое ИЛИ: ||
    void parseLogicalAnd();             //Логическое И: &&
    void parseEquality();               //Равенство: ==, !=
    void parseRelational();             //Сравнение: <, >, <=, >=
    void parseAdditive();               //Сложение и вычитание: +, -
    void parseMultiplicative();         //Умножение и деление: *, /
    void parseUnary();                  //Унарные операторы: !, -
    void parsePrimary();                //Базовые элементы
    

};