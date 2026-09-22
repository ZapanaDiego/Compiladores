#include "Lexer.h"
#include "SymbolTable.h"
#include <unordered_map>
#include <sstream>
#include <cctype>

std::optional<TokenType> Lexer::check_keyword(const std::string& lexeme) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KW_INT},
        {"float", TokenType::KW_FLOAT},
        {"char", TokenType::KW_CHAR},
        {"boolean", TokenType::KW_BOOLEAN},
        {"void", TokenType::KW_VOID},
        {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},
        {"for", TokenType::KW_FOR},
        {"while", TokenType::KW_WHILE},
        {"scanf", TokenType::KW_SCANF},
        {"println", TokenType::KW_PRINTLN},
        {"main", TokenType::KW_MAIN},
        {"return", TokenType::KW_RETURN}
    };

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return it->second;
    }
    return std::nullopt;
}

LexerOutput Lexer::analyze(const std::string& input, SymbolTable& symTable) {
    std::vector<Token> tokens;
    std::vector<LexerError> errors;

    const auto& chars = input;
    int len = static_cast<int>(chars.size());
    int i = 0;
    int line = 1;
    int col = 1;

    while (i < len) {
        char c = chars[i];

        if (c == '\n') {
            line++;
            col = 1;
            i++;
            continue;
        } else if (c == '\r' || c == ' ' || c == '\t') {
            col++;
            i++;
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            int start_col = col;
            std::string ident;

            while (i < len && (std::isalnum(static_cast<unsigned char>(chars[i])) || chars[i] == '_')) {
                ident += chars[i];
                i++;
                col++;
            }

            auto kw = check_keyword(ident);
            if (kw.has_value()) {
                tokens.push_back(Token{kw.value(), ident, -1, line, start_col});
            } else {
                int attr = symTable.insertOrGet(ident);
                tokens.push_back(Token{TokenType::ID, ident, attr, line, start_col});
            }
            continue;
        }

        if (c == '"') {
            int start_col = col;
            int start_line = line;
            std::string str_val;
            str_val += '"';
            i++;
            col++;
            bool closed = false;

            while (i < len) {
                char current = chars[i];
                if (current == '\n') break;
                if (current == '\\') {
                    str_val += '\\';
                    i++; col++;
                    if (i < len) {
                        str_val += chars[i];
                        i++; col++;
                    }
                    continue;
                }
                if (current == '"') {
                    str_val += '"';
                    i++; col++;
                    closed = true;
                    break;
                }
                str_val += current;
                i++; col++;
            }

            if (closed) {
                tokens.push_back(Token{TokenType::TEXTO, str_val, -1, start_line, start_col});
            } else {
                errors.push_back(LexerError{start_line, start_col, str_val, "Cadena de texto no cerrada"});
            }
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start_col = col;
            std::string num_str;
            bool is_decimal = false;

            while (i < len && std::isdigit(static_cast<unsigned char>(chars[i]))) {
                num_str += chars[i];
                i++; col++;
            }

            if (i < len && chars[i] == '.') {
                if (i + 1 < len && std::isdigit(static_cast<unsigned char>(chars[i + 1]))) {
                    is_decimal = true;
                    num_str += '.'; 
                    i++; col++;
                    while (i < len && std::isdigit(static_cast<unsigned char>(chars[i]))) {
                        num_str += chars[i];
                        i++; col++;
                    }
                }
            }

            TokenType ttype = is_decimal ? TokenType::NUM_DEC : TokenType::NUM_INT;
            tokens.push_back(Token{ttype, num_str, -1, line, start_col});
            continue;
        }

        errors.push_back(LexerError{line, col, std::string(1, c), "Simbolo no reconocido"});
        i++;
        col++;
    }

    return LexerOutput{tokens, errors, symTable.to_json()};
}
