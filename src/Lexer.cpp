#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& input) : input(input), pos(0), current_line(1), current_column(1) {}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return input[pos];
}

char Lexer::advance() {
    char c = input[pos++];
    if (c == '\n') {
        current_line++;
        current_column = 1;
    } else {
        current_column++;
    }
    return c;
}

bool Lexer::isAtEnd() {
    return pos >= input.length();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        std::size_t start_col = current_column;
        std::size_t start_line = current_line;
        char c = peek();

        if (std::isdigit(c)) {
            std::string lexeme;
            while (!isAtEnd() && std::isdigit(peek())) {
                lexeme += advance();
            }
            if (peek() == '.' && pos + 1 < input.length() && std::isdigit(input[pos + 1])) {
                lexeme += advance();
                while (!isAtEnd() && std::isdigit(peek())) {
                    lexeme += advance();
                }
            }
            tokens.push_back({TokenType::NUMBER, lexeme, start_line, start_col});

        } else if (std::isalpha(c) || c == '_') {
            std::string lexeme;
            while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
                lexeme += advance();
            }
            symbolTable.insert_or_update(lexeme, start_line, start_col);
            tokens.push_back({TokenType::IDENTIFIER, lexeme, start_line, start_col});

        } else if (c == '"') {
            std::string lexeme;
            lexeme += advance(); // comilla inicial
            bool closed = false;
            while (!isAtEnd()) {
                char ch = advance();
                lexeme += ch;
                if (ch == '"') { closed = true; break; }
            }
            tokens.push_back({closed ? TokenType::STRING : TokenType::ERROR, lexeme, start_line, start_col});

        } else {
            std::string lexeme(1, advance());
            tokens.push_back({TokenType::ERROR, lexeme, start_line, start_col});
        }
    }

    return tokens;
}