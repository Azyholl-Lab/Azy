#include "Lexer.h"
#include <unordered_map>

// Вспомогательная функция: проверка, является ли идентификатор ключевым словом
TokenType getKeywordType(const std::string& str) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var",      TokenType::KwLet},
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
        {"false",    TokenType::Bool}
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

    // Пропуск пробельных символов
    while (isspace(static_cast<unsigned char>(LastChar))) {
        LastChar = next_char(stream, current_line, current_col);
    }

    if (LastChar == EOF) {
        return {TokenType::EndOfFile, "", current_line, current_col};
    }

    int token_line = current_line;
    int token_col = current_col - 1;

    // 1. Распознавание идентификаторов и ключевых слов
    if (isalpha(static_cast<unsigned char>(LastChar)) || LastChar == '_') {
        std::string identifierStr;
        identifierStr += static_cast<char>(LastChar);
        while (isalnum(static_cast<unsigned char>(LastChar = next_char(stream, current_line, current_col))) || LastChar == '_') {
            identifierStr += static_cast<char>(LastChar);
        }
        
        TokenType type = getKeywordType(identifierStr);
        return {type, identifierStr, token_line, token_col};
    }

    // 2. Распознавание чисел (Int и Float)
    if (isdigit(static_cast<unsigned char>(LastChar)) || LastChar == '.') {
        std::string numStr;
        bool isFloat = false;

        // Если число начинается с точки (например, .5)
        if (LastChar == '.') {
            isFloat = true;
            numStr += '.';
            LastChar = next_char(stream, current_line, current_col);
            
            // Если после точки не цифра, это просто оператор точки (доступ к полям)
            if (!isdigit(static_cast<unsigned char>(LastChar))) {
                return {TokenType::Dot, ".", token_line, token_col};
            }
        }

        // Читаем целую часть (или цифры после начальной точки)
        while (isdigit(static_cast<unsigned char>(LastChar))) {
            numStr += static_cast<char>(LastChar);
            LastChar = next_char(stream, current_line, current_col);
        }

        // Если встретили точку после целой части (например, 3.14)
        if (!isFloat && LastChar == '.') {
            isFloat = true;
            numStr += '.';
            LastChar = next_char(stream, current_line, current_col);

            // Читаем дробную часть
            while (isdigit(static_cast<unsigned char>(LastChar))) {
                numStr += static_cast<char>(LastChar);
                LastChar = next_char(stream, current_line, current_col);
            }
        }

        TokenType type = isFloat ? TokenType::Float : TokenType::Int;
        return {type, numStr, token_line, token_col};
    }

    // 3. Обработка комментариев (#, //, /* ... */)
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
                return {TokenType::Unknown, "", token_line, token_col};
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

    // 4. Строки в кавычках
    if (LastChar == '"') {
        std::string strVal;
        LastChar = next_char(stream, current_line, current_col); 
        while (LastChar != EOF && LastChar != '"') {
            strVal += static_cast<char>(LastChar);
            LastChar = next_char(stream, current_line, current_col);
        }
        if (LastChar == '"') {
            LastChar = next_char(stream, current_line, current_col); 
        }
        return {TokenType::String, strVal, token_line, token_col};
    }

    // 5. Операторы, составные знаки и разделители
    char c = static_cast<char>(LastChar);
    LastChar = next_char(stream, current_line, current_col); 

    TokenType type = TokenType::Unknown;
    std::string opStr(1, c);

    // Проверка составных операторов
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
    
    // Одинарные операторы и разделители
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

    return {type, opStr, token_line, token_col};
}