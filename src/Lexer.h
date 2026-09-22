#pragma once
#include <string>
#include <vector>

#include "Token.h"
#include "SymbolTable.h"

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();
    const SymbolTable& getSymbolTable() const { return symbolTable; }

private:
    std::string input;
    size_t pos;
    int current_line;
    int current_column;
    SymbolTable symbolTable;

    char peek();
    char advance();
    bool isAtEnd();
    void skipWhitespace();
};
