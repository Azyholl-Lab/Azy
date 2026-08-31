#include <iostream>
#include "Token.h"
#include "lexer.h"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Int:        return "Int";
        case TokenType::Float:      return "Float";
        case TokenType::Operator:   return "Operator";
        case TokenType::String:     return "String";
        case TokenType::Unknown:    return "Unknown";
    }
    return "Unknown";
}

void processToken(const Token& tok) {
    if(tok.type != TokenType::Unknown){
    std::cout << "[Парсер получил] Тип: " << tokenTypeToString(tok.type)
              << ", Значение: \"" << tok.value 
              << "\", Строка: " << tok.line 
              << ", Символ: " << tok.column << "\n";
}
    else {
        std::cout << ", Значение: \"" << tok.value << "\" Ошибка в строке: "  << tok.line  << ", символ: " << tok.column << "\n";
    }
}

int main(int argc, char *argv[]) {
    FILE *input = stdin; 

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Ошибка открытия файла");
            return 1;
        }
    }

    std::cout << "Главный файл запущен. Читаем токены...\n";

    while (true) {
        if (feof(input)) break;

        Token tok = gettok(input);
        
        if (tok.value.empty() && tok.type == TokenType::Unknown) {
            break;
        }

        processToken(tok);
    }

    if (input != stdin) {
        fclose(input);
    }
    return 0;
}