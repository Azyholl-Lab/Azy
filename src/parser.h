#pragma once
#include <vector>
#include "Token.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t currentPos = 0;

    // Вспомогательные методы для навигации по токенам
    Token peek();         // Посмотреть текущий токен без сдвига
    Token consume();      // Взять текущий токен и сдвинуть указатель вперед
    bool check(TokenType type, const std::string& value); // Проверить текущий токен

public:
    Parser(const std::vector<Token>& toks);
    void parse();         // Главный метод запуска

    // Функции-обработчики для разных конструкций (ветвления)
    void parseStatement();      // точка входа
    void parseBlock();          // тело блока
    
    void parseIf();             // блок if
    void parseElse();           // блок else
    void parseExpression();     // разобрать выражение
    // void parseWhile();
    // void parseFor();
    // void parseFunc();
    
    
    void parseFactor();

    void parserError(const Token& token, const std::string& message);
};