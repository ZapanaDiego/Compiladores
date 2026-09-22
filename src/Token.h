#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <sstream>
#include "SymbolTable.h"

enum class TokenType {
    NUM_INT, NUM_DEC, ID, TEXTO,
    KW_INT, KW_FLOAT, KW_CHAR, KW_BOOLEAN, KW_VOID,
    KW_IF, KW_ELSE, KW_FOR, KW_WHILE,
    KW_SCANF, KW_PRINTLN, KW_MAIN, KW_RETURN,
    UNKNOWN, END_OF_FILE
};

inline std::string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::NUM_INT: return "NUM_INT";
        case TokenType::NUM_DEC: return "NUM_DEC";
        case TokenType::ID: return "ID";
        case TokenType::TEXTO: return "TEXTO";
        case TokenType::KW_INT: return "KW_INT";
        case TokenType::KW_FLOAT: return "KW_FLOAT";
        case TokenType::KW_CHAR: return "KW_CHAR";
        case TokenType::KW_BOOLEAN: return "KW_BOOLEAN";
        case TokenType::KW_VOID: return "KW_VOID";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_FOR: return "KW_FOR";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_SCANF: return "KW_SCANF";
        case TokenType::KW_PRINTLN: return "KW_PRINTLN";
        case TokenType::KW_MAIN: return "KW_MAIN";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::UNKNOWN: return "UNKNOWN";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        default: return "UNKNOWN";
    }
}

struct Token {
    TokenType type;
    std::string lexeme;
    int attribute = -1;
    int line;
    int col;

    std::string toString() const {
        if (type == TokenType::ID) {
            return "<ID, " + std::to_string(attribute) + ">";
        }
        return "<" + tokenTypeToString(type) + ">";
    }

    std::string to_json() const {
        std::ostringstream oss;
        std::string attr_json = (attribute != -1) ? std::to_string(attribute) : "null";
        oss << "{\"type\":\"" << escape_json(tokenTypeToString(type))
            << "\",\"lexeme\":\"" << escape_json(lexeme)
            << "\",\"attr\":" << attr_json
            << ",\"line\":" << line
            << ",\"col\":" << col << "}";
        return oss.str();
    }

private:
    static std::string escape_json(const std::string& s) {
        std::string result;
        result.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n";  break;
                case '\r': result += "\\r";  break;
                case '\t': result += "\\t";  break;
                default:   result += c;      break;
            }
        }
        return result;
    }
};

struct LexerError {
    int line;
    int col;
    std::string lexeme;
    std::string message;

    std::string to_json() const {
        std::ostringstream oss;
        oss << "{\"line\":" << line
            << ",\"col\":" << col
            << ",\"lexeme\":\"" << escape_json(lexeme)
            << "\",\"message\":\"" << escape_json(message) << "\"}";
        return oss.str();
    }

private:
    static std::string escape_json(const std::string& s) {
        std::string result;
        result.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n";  break;
                case '\r': result += "\\r";  break;
                case '\t': result += "\\t";  break;
                default:   result += c;      break;
            }
        }
        return result;
    }
};

struct LexerOutput {
    std::vector<Token> tokens;
    std::vector<LexerError> errors;
    std::string symbols_json;

    std::string to_json() const {
        std::ostringstream oss;
        oss << "{\"tokens\":[";
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) oss << ",";
            oss << tokens[i].to_json();
        }
        oss << "],\"errors\":[";
        for (size_t i = 0; i < errors.size(); ++i) {
            if (i > 0) oss << ",";
            oss << errors[i].to_json();
        }
        oss << "],\"symbols\":" << (symbols_json.empty() ? "[]" : symbols_json) << "}";
        return oss.str();
    }
};

#endif // TOKEN_H
