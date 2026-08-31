#include <iostream>
#include "lexer.h"

// Пример функции, куда мы передаем токен дальше (например, парсер)
void processToken(const Token& tok) {
    // Здесь ваша логика обработки токена (синтаксический анализ и т.д.)
    std::cout << "[Парсер получил] Токен со значением: " << tok.value << "\n";
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

    // Главный цикл сбора токенов
    while (true) {
        if (feof(input)) break;

        Token tok = gettok(input);
        
        // Если дошли до конца или пустой токен — прерываемся
        if (tok.value.empty() && tok.type == TokenType::Unknown) {
            break;
        }

        // ПЕРЕДАЕМ ТОКЕН ДАЛЬШЕ
        processToken(tok);
    }

    if (input != stdin) {
        fclose(input);
    }
    return 0;
}