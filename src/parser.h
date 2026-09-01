#pragma once
#include <vector>
#include "Token.h"

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

public:
    Parser(const std::vector<Token>& toks);
    void parse();         // Главный метод запуска

    void parserError(const Token& token, const std::string& message);

    // Функции-обработчики для разных конструкций (ветвления)
    void parseStatement();      // точка входа
    
    void parseIf();
    void parseElse();
    
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