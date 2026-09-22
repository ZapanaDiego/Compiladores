#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <optional>

class Lexer {
public:
    static std::optional<TokenType> check_keyword(const std::string& lexeme);
    static LexerOutput analyze(const std::string& input, SymbolTable& symTable);
};

#endif // LEXER_H
