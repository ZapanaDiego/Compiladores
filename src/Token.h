#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <sstream>

// Equivalente a token.rs

struct Token {
    std::string type;    // "NUM_INT", "NUM_DEC", "ID", "KW_*", "STRING", etc.
    std::string lexeme;  // p.ej. "20" o "15.5"
    int line;
    int col;

    std::string to_json() const {
        std::ostringstream oss;
        oss << "{\"type\":\"" << escape_json(type)
            << "\",\"lexeme\":\"" << escape_json(lexeme)
            << "\",\"line\":" << line
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
        oss << "]}";
        return oss.str();
    }
};

#endif // TOKEN_H
