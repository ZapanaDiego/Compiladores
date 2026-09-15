#pragma once
#include <string>
#include <vector>

#include "Token.h"

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos;
    int current_line;
    int current_column;

    char peek();
    char advance();
    bool isAtEnd();
    void skipWhitespace();
};
