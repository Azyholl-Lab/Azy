#include <iostream>
#include <vector>
#include <string>
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Semantic.h"
#include "Interpreter.h"

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    bool showAst = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--ast") { showAst = true; continue; }
        if (input == stdin && arg.size() >= 4 &&
            arg.compare(arg.size() - 4, 4, ".azy") == 0) {
            input = fopen(arg.c_str(), "r");
            if (!input) { perror("Ошибка открытия файла"); return 1; }
        } else if (input == stdin) {
            std::cerr << "[Ошибка] Ожидался файл с расширением .azy\n";
            return 1;
        }
    }

    try {
        std::vector<Token> tokens;
        while (true) {
            Token tok = gettok(input);
            if (tok.type == TokenType::EndOfFile) { tokens.push_back(tok); break; }
            if (tok.type != TokenType::Unknown) tokens.push_back(tok);
        }
        if (input != stdin) { fclose(input); input = nullptr; }

        Parser parser(tokens);
        auto root = parser.parse();

        if (showAst) {
            std::cout << "=== AST ===\n";
            parser.printAST(root.get());
            std::cout << "===========\n\n";
        }

        SemanticAnalyzer semantic;
        auto errors = semantic.analyze(root.get());
        if (!errors.empty()) {
            std::cerr << "=== Семантические ошибки (" << errors.size() << ") ===\n\n";
            for (const auto& e : errors) std::cerr << e << "\n\n";
            return 1;
        }

        std::cout << "=== Запуск ===\n";
        Interpreter interp;
        interp.execute(root.get());
        std::cout << "\n=== Завершено ===\n";
    }
    catch (const std::exception& e) {
        if (input != nullptr && input != stdin) fclose(input);
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}