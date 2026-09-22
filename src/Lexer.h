#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <optional>

// Equivalente a lexer.rs

class Lexer {
public:
    /// Determina si un lexema es una palabra reservada y devuelve su tipo de token.
    static std::optional<std::string> check_keyword(const std::string& lexeme);

    /// Ejecuta el análisis léxico sobre el texto de entrada.
    static LexerOutput analyze(const std::string& input);
};

#endif // LEXER_H
