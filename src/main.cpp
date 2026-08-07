#include <iostream>
#include <fstream>
#include <string>

// Функция для чтения файла исходного кода нашего языка
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filepath << std::endl;
        exit(1);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char* argv[]) {
    // Проверяем, передал ли пользователь файл нашему компилятору
    if (argc < 2) {
        std::cout << "Использование: AZY <файл.ml>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::cout << "Читаем файл: " << filename << std::endl;

    std::string sourceCode = readFile(filename);
    std::cout << "Содержимое файла:\n" << sourceCode << std::endl;

    // Здесь в будущем будет вызов лексера, парсера и интерпретатора!

    return 0;
}