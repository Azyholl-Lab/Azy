#pragma once
#include <string>
#include <unordered_map>
#include "Token.h"

enum class ErrCategory {
    Lexer,
    Parser,
    Semantic,
    Runtime,
    Warning,
};

enum class ErrCode {
    // --- Lexer errors ---
    L001_UnknownChar,
    L002_UnterminatedString,
    L003_UnterminatedComment,
    L004_UnterminatedMultiLineString,

    // --- Lexer warnings ---
    W001_MultiLineString,

    // --- Parser general ---
    P001_ExpectedSemicolon,
    P002_ExpectedOpeningParen,
    P003_ExpectedClosingParen,
    P004_ExpectedOpeningBrace,
    P005_ExpectedClosingBrace,
    P006_ExpectedIdentifier,
    P007_ExpectedExpression,
    P008_ExpectedModifier,
    P009_UnexpectedToken,
    P010_ExpectedAssign,
    P011_ExpectedForStep,

    // --- Parser specific ---
    P100_UnknownForForm,
    P101_InputEmpty,
    P102_InputWithArgsAsExpr,
    P103_EmptyVarDecl,
    P104_MissingWhileAfterDo,

    // --- Semantic ---
    S001_UndeclaredVariable,
    S002_DuplicateDeclaration,
    S003_ReturnOutsideFunction,
    S004_BreakOutsideLoop,
    S005_ContinueOutsideLoop,
    S006_TypeMismatch,
    S007_NotCallable,

    // --- Runtime ---
    R001_DivisionByZero,
    R002_InputReadFailed,
};

struct ErrorInfo {
    const char* code;
    ErrCategory category;
    const char* message;
    const char* hint;
};

const ErrorInfo& getErrorInfo(ErrCode code);

std::string formatError(ErrCode code, const Token& tok,
                        const std::string& detail = "");

std::string formatError(ErrCode code, int line, int column,
                        const std::string& tokenValue,
                        const std::string& detail = "");

std::string formatWarning(ErrCode code, int line, int column,
                          const std::string& tokenValue = "",
                          const std::string& detail = "");

std::string formatCategory(ErrCategory cat);