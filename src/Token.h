#pragma once
#include <string>
#include <cstddef>

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    STRING,
    OPERATOR,
    PUNCTUATION,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};

struct LexerError {
    std::string message;
    std::size_t line;
    std::size_t column;
};