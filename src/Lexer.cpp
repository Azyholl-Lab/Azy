#include "Lexer.h"
#include "Errors.h"
#include <unordered_map>
#include <stdexcept>
#include <iostream>

TokenType getKeywordType(const std::string& str) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var",      TokenType::KwVar},
        {"fn",       TokenType::KwFn},
        {"def",      TokenType::KwFn},
        {"if",       TokenType::KwIf},
        {"else",     TokenType::KwElse},
        {"while",    TokenType::KwWhile},
        {"do",       TokenType::KwDo},
        {"for",      TokenType::KwFor},
        {"in",       TokenType::KwIn},
        {"return",   TokenType::KwReturn},
        {"break",    TokenType::KwBreak},
        {"continue", TokenType::KwContinue},
        {"not",      TokenType::KwNot},
        {"true",     TokenType::Bool},
        {"false",    TokenType::Bool},
        {"print",    TokenType::KwPrint},
        {"input",    TokenType::KwInput},
        {"int",      TokenType::KwInt},
        {"float",    TokenType::KwFloat},
        {"char",     TokenType::KwChar},
        {"string",   TokenType::KwString}
    };

    auto it = keywords.find(str);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::Identifier;
}

int next_char(FILE *stream, int &line, int &column) {
    int c = fgetc(stream);
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

int peek_next(FILE *stream) {
    int next = fgetc(stream);
    if (next != EOF) {
        ungetc(next, stream);
    }
    return next;
}

Token gettok(FILE *stream) {
    static int LastChar = ' ';
    static int current_line = 1;
    static int current_col = 1;
    static bool bomChecked = false;

    // Пропуск UTF-8 BOM
    if (!bomChecked) {
        bomChecked = true;
        int b1 = fgetc(stream);
        int b2 = fgetc(stream);
        int b3 = fgetc(stream);
        bool isBom = (b1 == 0xEF && b2 == 0xBB && b3 == 0xBF);
        if (!isBom) {
            if (b3 != EOF) ungetc(b3, stream);
            if (b2 != EOF) ungetc(b2, stream);
            if (b1 != EOF) ungetc(b1, stream);
        }
    }

    while (isspace(static_cast<unsigned char>(LastChar))) {
        LastChar = next_char(stream, current_line, current_col);
    }

    if (LastChar == EOF) {
        return {TokenType::EndOfFile, "", current_line, current_col};
    }

    int token_line = current_line;
    int token_col = current_col - 1;

    // 1. Идентификаторы
    if (isalpha(static_cast<unsigned char>(LastChar)) || LastChar == '_') {
        std::string identifierStr;
        identifierStr += static_cast<char>(LastChar);
        while (isalnum(static_cast<unsigned char>(LastChar = next_char(stream, current_line, current_col))) || LastChar == '_') {
            identifierStr += static_cast<char>(LastChar);
        }
        TokenType type = getKeywordType(identifierStr);
        return {type, identifierStr, token_line, token_col};
    }

    // 2. Числа
    if (isdigit(static_cast<unsigned char>(LastChar)) || LastChar == '.') {
        std::string numStr;
        bool isFloat = false;

        if (LastChar == '.') {
            isFloat = true;
            numStr += '.';
            LastChar = next_char(stream, current_line, current_col);
            if (!isdigit(static_cast<unsigned char>(LastChar))) {
                return {TokenType::Dot, ".", token_line, token_col};
            }
        }
        while (isdigit(static_cast<unsigned char>(LastChar))) {
            numStr += static_cast<char>(LastChar);
            LastChar = next_char(stream, current_line, current_col);
        }
        if (!isFloat && LastChar == '.') {
            isFloat = true;
            numStr += '.';
            LastChar = next_char(stream, current_line, current_col);
            while (isdigit(static_cast<unsigned char>(LastChar))) {
                numStr += static_cast<char>(LastChar);
                LastChar = next_char(stream, current_line, current_col);
            }
        }
        TokenType type = isFloat ? TokenType::Float : TokenType::Int;
        return {type, numStr, token_line, token_col};
    }

    // 3. Комментарии
    if (LastChar == '#' || (LastChar == '/' && peek_next(stream) == '/')) {
        do {
            LastChar = next_char(stream, current_line, current_col);
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) {
            return gettok(stream);
        }
        return {TokenType::EndOfFile, "", token_line, token_col};
    }

    if (LastChar == '/' && peek_next(stream) == '*') {
        LastChar = next_char(stream, current_line, current_col);
        LastChar = next_char(stream, current_line, current_col);
        while (true) {
            if (LastChar == EOF) {
                throw std::runtime_error(formatError(
                    ErrCode::L003_UnterminatedComment,
                    token_line, token_col, "/*"));
            }
            if (LastChar == '*' && peek_next(stream) == '/') {
                LastChar = next_char(stream, current_line, current_col);
                LastChar = next_char(stream, current_line, current_col);
                break;
            }
            LastChar = next_char(stream, current_line, current_col);
        }
        return gettok(stream);
    }

    // 4. Строки в двойных кавычках
    if (LastChar == '"') {
        std::string strVal;
        LastChar = next_char(stream, current_line, current_col);
        while (LastChar != EOF && LastChar != '"') {
            strVal += static_cast<char>(LastChar);
            LastChar = next_char(stream, current_line, current_col);
        }

        bool hasNewline = (strVal.find('\n') != std::string::npos);

        if (LastChar == EOF) {
            // Строка не закрылась до конца файла.
            // Если внутри уже были переводы строк — это классический случай
            // «забыта кавычка, а следующая '"' с другой строки её съела».
            ErrCode code = hasNewline
                ? ErrCode::L004_UnterminatedMultiLineString
                : ErrCode::L002_UnterminatedString;

            std::string shortVal = strVal.substr(0, 25);
            if (strVal.length() > 25) shortVal += "...";

            throw std::runtime_error(formatError(
                code, token_line, token_col, "\"" + shortVal));
        }

        // Строка успешно закрылась — но содержит переводы строк.
        // Это подозрительно: печатаем предупреждение с АДРЕСОМ НАЧАЛА.
        if (hasNewline) {
            std::string shortVal = strVal.substr(0, 25);
            if (strVal.length() > 25) shortVal += "...";

            std::cerr << formatWarning(
                ErrCode::W001_MultiLineString,
                token_line, token_col, "\"" + shortVal) << "\n";
        }

        LastChar = next_char(stream, current_line, current_col);  // закрывающая '"'
        return {TokenType::String, strVal, token_line, token_col};
    }

    // 5. Операторы, составные знаки и разделители
    char c = static_cast<char>(LastChar);
    LastChar = next_char(stream, current_line, current_col);

    TokenType type = TokenType::Unknown;
    std::string opStr(1, c);

    if (c == '=' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::Equal; }
    else if (c == '!' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::NotEqual; }
    else if (c == '<' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::LessEq; }
    else if (c == '>' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::GreaterEq; }
    else if (c == '&' && LastChar == '&') { opStr += '&'; LastChar = next_char(stream, current_line, current_col); type = TokenType::And; }
    else if (c == '|' && LastChar == '|') { opStr += '|'; LastChar = next_char(stream, current_line, current_col); type = TokenType::Or; }
    else if (c == '+' && LastChar == '+') { opStr += '+'; LastChar = next_char(stream, current_line, current_col); type = TokenType::Increment; }
    else if (c == '+' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::PlusAssign; }
    else if (c == '-' && LastChar == '-') { opStr += '-'; LastChar = next_char(stream, current_line, current_col); type = TokenType::Decrement; }
    else if (c == '-' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::MinusAssign; }
    else if (c == '*' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::MultAssign; }
    else if (c == '/' && LastChar == '=') { opStr += '='; LastChar = next_char(stream, current_line, current_col); type = TokenType::DivAssign; }
    else {
        switch (c) {
            case '+': type = TokenType::Plus; break;
            case '-': type = TokenType::Minus; break;
            case '*': type = TokenType::Multiply; break;
            case '/': type = TokenType::Divide; break;
            case '=': type = TokenType::Assign; break;
            case '<': type = TokenType::Less; break;
            case '>': type = TokenType::Greater; break;
            case '!': type = TokenType::NotOp; break;
            case '(': type = TokenType::LParen; break;
            case ')': type = TokenType::RParen; break;
            case '{': type = TokenType::LBrace; break;
            case '}': type = TokenType::RBrace; break;
            case '[': type = TokenType::LBracket; break;
            case ']': type = TokenType::RBracket; break;
            case ';': type = TokenType::Semicolon; break;
            case ',': type = TokenType::Comma; break;
            case ':': type = TokenType::Colon; break;
            case '.': type = TokenType::Dot; break;
            default:  type = TokenType::Unknown; break;
        }
    }

    if (type == TokenType::Unknown) {
        std::string detail = "'" + std::string(1, c) + "'";
        if (c == '&') detail += " (возможно, вы имели в виду '&&' для логического И)";
        if (c == '|') detail += " (возможно, вы имели в виду '||' для логического ИЛИ)";
        throw std::runtime_error(formatError(
            ErrCode::L001_UnknownChar,
            token_line, token_col, "", detail));
    }

    return {type, opStr, token_line, token_col};
}