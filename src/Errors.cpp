#include "Errors.h"
#include <sstream>
#include <stdexcept>

static const std::unordered_map<ErrCode, ErrorInfo> TABLE = {
    // ===== Лексер =====
    {ErrCode::L001_UnknownChar, {
        "L001", ErrCategory::Lexer,
        "Неизвестный символ", ""}},
    {ErrCode::L002_UnterminatedString, {
        "L002", ErrCategory::Lexer,
        "Незакрытая строка", "Добавьте закрывающую '\"' в конце строки"}},
    {ErrCode::L003_UnterminatedComment, {
        "L003", ErrCategory::Lexer,
        "Незакрытый блочный комментарий",
        "Добавьте '*/' для закрытия комментария, начатого '/*'"}},

    // ===== Парсер — общие =====
    {ErrCode::P001_ExpectedSemicolon, {
        "P001", ErrCategory::Parser,
        "Ожидался терминатор ';'", ""}},
    {ErrCode::P002_ExpectedOpeningParen, {
        "P002", ErrCategory::Parser,
        "Ожидалась открывающая скобка '('", ""}},
    {ErrCode::P003_ExpectedClosingParen, {
        "P003", ErrCategory::Parser,
        "Ожидалась закрывающая скобка ')'", ""}},
    {ErrCode::P004_ExpectedOpeningBrace, {
        "P004", ErrCategory::Parser,
        "Ожидался блок кода '{ ... }'", ""}},
    {ErrCode::P005_ExpectedClosingBrace, {
        "P005", ErrCategory::Parser,
        "Ожидалась закрывающая фигурная скобка '}'", ""}},
    {ErrCode::P006_ExpectedIdentifier, {
        "P006", ErrCategory::Parser,
        "Ожидался идентификатор", ""}},
    {ErrCode::P007_ExpectedExpression, {
        "P007", ErrCategory::Parser,
        "Ожидалось выражение", ""}},
    {ErrCode::P008_ExpectedModifier, {
        "P008", ErrCategory::Parser,
        "Ожидался модификатор после '.'", ""}},
    {ErrCode::P009_UnexpectedToken, {
        "P009", ErrCategory::Parser,
        "Неожиданный токен", ""}},
    {ErrCode::P010_ExpectedAssign, {
        "P010", ErrCategory::Parser,
        "Ожидался оператор присваивания '='", ""}},
    {ErrCode::P011_ExpectedForStep, {
        "P011", ErrCategory::Parser,
        "Ожидался '++', '--' или '=' в шаге for", ""}},

    // ===== Парсер — конкретные =====
    {ErrCode::P100_UnknownForForm, {
        "P100", ErrCategory::Parser,
        "Неизвестная форма for: после переменной ожидалось '=', '==' или 'in'",
        "Примеры: for (i == 5) {}, for (i = 0; i == 5; i++) {}, for (x in str) {}"}},
    {ErrCode::P101_InputEmpty, {
        "P101", ErrCategory::Parser,
        "input без переменных не является оператором",
        "Используйте как выражение: x = input(); или x = input.line();"}},
    {ErrCode::P102_InputWithArgsAsExpr, {
        "P102", ErrCategory::Parser,
        "input с аргументами не является выражением",
        "Используйте как оператор: input(var); или input(var1, var2);"}},
    {ErrCode::P103_EmptyVarDecl, {
        "P103", ErrCategory::Parser,
        "Пустое объявление переменной после 'var'",
        "Используйте: var x = 5; или var x;"}},
    {ErrCode::P104_MissingWhileAfterDo, {
        "P104", ErrCategory::Parser,
        "Ожидалось 'while' после блока do { ... }",
        "Правильно: do { ... } while (условие);"}},

    // ===== Семантика =====
    {ErrCode::S001_UndeclaredVariable, {
        "S001", ErrCategory::Semantic,
        "Использование необъявленной переменной",
        "Объявите её через var до первого использования"}},
    {ErrCode::S002_DuplicateDeclaration, {
        "S002", ErrCategory::Semantic,
        "Повторное объявление переменной в той же области видимости", ""}},
    {ErrCode::S003_ReturnOutsideFunction, {
        "S003", ErrCategory::Semantic,
        "'return' вне тела функции", ""}},
    {ErrCode::S004_BreakOutsideLoop, {
        "S004", ErrCategory::Semantic,
        "'break' вне цикла", ""}},
    {ErrCode::S005_ContinueOutsideLoop, {
        "S005", ErrCategory::Semantic,
        "'continue' вне цикла", ""}},
    {ErrCode::S006_TypeMismatch, {
        "S006", ErrCategory::Semantic,
        "Несовместимые типы", ""}},
    {ErrCode::S007_NotCallable, {
        "S007", ErrCategory::Semantic,
        "Попытка вызвать не-функцию", ""}},

    // ===== Runtime =====
    {ErrCode::R001_DivisionByZero, {
        "R001", ErrCategory::Runtime,
        "Деление на ноль", ""}},
    {ErrCode::R002_InputReadFailed, {
        "R002", ErrCategory::Runtime,
        "Не удалось прочитать значение из input", ""}},
};

const ErrorInfo& getErrorInfo(ErrCode code) {
    auto it = TABLE.find(code);
    if (it == TABLE.end()) {
        throw std::logic_error(
            "Внутренняя ошибка Azy: ErrCode отсутствует в таблице ошибок");
    }
    return it->second;
}

std::string formatCategory(ErrCategory cat) {
    switch (cat) {
        case ErrCategory::Lexer:    return "Ошибка лексера";
        case ErrCategory::Parser:   return "Ошибка парсера";
        case ErrCategory::Semantic: return "Ошибка семантики";
        case ErrCategory::Runtime:  return "Ошибка выполнения";
    }
    return "Ошибка";
}

static std::string buildMessage(const ErrorInfo& info,
                                int line, int column,
                                const std::string& tokenValue,
                                const std::string& detail) {
    std::ostringstream out;
    out << "[" << formatCategory(info.category) << "] " << info.code << ": " << info.message;
    if (!detail.empty()) {
        out << " " << detail;
    }
    if (info.hint != nullptr && info.hint[0] != '\0') {
        out << "\n  Подсказка: " << info.hint;
    }
    out << "\n  (строка: " << line << ", символ: " << column;
    if (!tokenValue.empty()) {
        out << ", токен: '" << tokenValue << "'";
    }
    out << ")";
    return out.str();
}

std::string formatError(ErrCode code, const Token& tok, const std::string& detail) {
    return formatError(code, tok.line, tok.column, tok.value, detail);
}

std::string formatError(ErrCode code, int line, int column, const std::string& tokenValue, const std::string& detail) {
    return buildMessage(getErrorInfo(code), line, column, tokenValue, detail);
}