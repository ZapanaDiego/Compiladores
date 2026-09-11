#pragma once
#include <string>
#include <vector>

struct Token {
    std::string type;
    std::string lexeme;
    int line;
    int column;
};

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
