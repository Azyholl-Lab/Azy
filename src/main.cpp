#include <iostream>
#include <vector>
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Semantic.h"

int main(int argc, char *argv[]) {
    FILE *input = stdin;

    if (argc > 1) {
        std::string filename = argv[1];
        if (filename.length() < 4 ||
            filename.compare(filename.length() - 4, 4, ".azy") != 0) {
            std::cerr << "[Ошибка] Поддерживаются только файлы с расширением .azy\n";
            return 1;
        }
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Ошибка открытия файла");
            return 1;
        }
    }

    try {
        std::cout << "Главный файл запущен. Читаем токены...\n";
        std::vector<Token> tokens;

        while (true) {
            Token tok = gettok(input);
            if (tok.type == TokenType::EndOfFile) {
                tokens.push_back(tok);
                break;
            }
            if (tok.type != TokenType::Unknown) {
                tokens.push_back(tok);
            }
        }

        if (input != stdin) { fclose(input); input = nullptr; }

        std::cout << "Лексер закончил свою работу\n";
        std::cout << "Всего сохранено токенов в массив: " << tokens.size() << "\n\n\n";
        std::cout << "Запущен парсер...\n";

        Parser parser(tokens);
        auto root = parser.parse();

        std::cout << "Готовое дерево AST: \n\n";
        parser.printAST(root.get());
        std::cout << "Парсер закончил свою работу\n\n";

        // Семантический анализ
        std::cout << "Запущен семантический анализ...\n";
        SemanticAnalyzer semantic;
        auto errors = semantic.analyze(root.get());

        if (!errors.empty()) {
            std::cerr << "\n=== Семантические ошибки (" << errors.size() << ") ===\n\n";
            for (const auto& e : errors) {
                std::cerr << e << "\n\n";
            }
            return 1;
        }

        std::cout << "Семантика: OK\n";
    }
    catch (const std::exception& e) {
        if (input != nullptr && input != stdin) fclose(input);
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}