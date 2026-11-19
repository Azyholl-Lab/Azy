#pragma once
#include <string>
#include <unordered_map>
#include "Token.h"

// Категория ошибки — для группировки, цвета в IDE, фильтрации тестов
enum class ErrCategory {
    Lexer,
    Parser,
    Semantic,
    Runtime,
};

// Код ошибки. Префиксы:
//   L — лексер, P — парсер, S — семантика, R — runtime
enum class ErrCode {
    // ===== Лексер =====
    L001_UnknownChar,            // неизвестный символ
    L002_UnterminatedString,     // незакрытая строка
    L003_UnterminatedComment,    // незакрытый /* ... */

    // ===== Парсер — общие ожидания =====
    P001_ExpectedSemicolon,      // ожидался ';'
    P002_ExpectedOpeningParen,   // ожидался '('
    P003_ExpectedClosingParen,   // ожидался ')'
    P004_ExpectedOpeningBrace,   // ожидался '{'
    P005_ExpectedClosingBrace,   // ожидался '}'
    P006_ExpectedIdentifier,     // ожидался идентификатор
    P007_ExpectedExpression,     // ожидалось выражение
    P008_ExpectedModifier,       // ожидался модификатор после '.'
    P009_UnexpectedToken,        // неожиданный токен
    P010_ExpectedAssign,         // ожидался '='
    P011_ExpectedForStep,        // ожидался '++' / '--' / '='

    // ===== Парсер — конкретные =====
    P100_UnknownForForm,         // неизвестная форма for
    P101_InputEmpty,             // input() без переменных
    P102_InputWithArgsAsExpr,    // input(x) как выражение
    P103_EmptyVarDecl,           // var без имени
    P104_MissingWhileAfterDo,    // do ... без while

    // ===== Семантика  =====
    S001_UndeclaredVariable,
    S002_DuplicateDeclaration,
    S003_ReturnOutsideFunction,
    S004_BreakOutsideLoop,
    S005_ContinueOutsideLoop,
    S006_TypeMismatch,
    S007_NotCallable,

    // ===== Runtime  =====
    R001_DivisionByZero,
    R002_InputReadFailed,
};

// Запись в таблице ошибок
struct ErrorInfo {
    const char* code;       // "P002"
    ErrCategory category;   // Parser
    const char* message;    // "Ожидалась открывающая скобка '('"
    const char* hint;       // "..." или "" — расширенная подсказка
};

// Получить запись по коду (бросает std::logic_error, если код не в таблице)
const ErrorInfo& getErrorInfo(ErrCode code);

// Собрать финальное сообщение:
std::string formatError(ErrCode code, const Token& tok,
                        const std::string& detail = "");
std::string formatError(ErrCode code, int line, int column,
                        const std::string& tokenValue,
                        const std::string& detail = "");

// "Ошибка парсера" / "Ошибка лексера" / ...
std::string formatCategory(ErrCategory cat);