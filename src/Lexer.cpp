<<<<<<< HEAD
#include "lexer.h"
#include <unordered_map>
#include <sstream>

// Equivalente a lexer.rs — Lexer::check_keyword y Lexer::analyze

/// Determina si un lexema es una palabra reservada y devuelve su tipo de token.
std::optional<std::string> Lexer::check_keyword(const std::string& lexeme) {
    static const std::unordered_map<std::string, std::string> keywords = {
        // Estructuras de control
        {"if",       "KW_IF"},
        {"else",     "KW_ELSE"},
        {"while",    "KW_WHILE"},
        {"for",      "KW_FOR"},
        {"do",       "KW_DO"},
        {"return",   "KW_RETURN"},
        {"break",    "KW_BREAK"},
        {"continue", "KW_CONTINUE"},
        {"switch",   "KW_SWITCH"},
        {"case",     "KW_CASE"},
        {"default",  "KW_DEFAULT"},

        // Declaraciones y tipos de datos
        {"let",      "KW_LET"},
        {"var",      "KW_VAR"},
        {"const",    "KW_CONST"},
        {"fn",       "KW_FN"},
        {"function", "KW_FUNCTION"},
        {"int",      "KW_INT"},
        {"float",    "KW_FLOAT"},
        {"double",   "KW_DOUBLE"},
        {"char",     "KW_CHAR"},
        {"string",   "KW_STRING"},
        {"bool",     "KW_BOOL"},
        {"void",     "KW_VOID"},
        {"struct",   "KW_STRUCT"},
        {"class",    "KW_CLASS"},

        // Literales booleanos / nulos
        {"true",     "KW_TRUE"},
        {"false",    "KW_FALSE"},
        {"null",     "KW_NULL"},

        // Funciones / identificadores clave comunes
        {"print",    "KW_PRINT"},
        {"println",  "KW_PRINTLN"},
        {"main",     "KW_MAIN"},
    };

=======
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

>>>>>>> main
    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return it->second;
    }
    return std::nullopt;
}

<<<<<<< HEAD
/// Ejecuta el análisis léxico sobre el texto de entrada.
LexerOutput Lexer::analyze(const std::string& input) {
=======
static inline bool is_token_boundary(char ch) {
    switch (ch) {
        case ';': case ',':
        case '(': case ')':
        case '{': case '}':
        case '[': case ']':
        case '+': case '-':
        case '*': case '/':
        case '%': case '=':
        case '<': case '>':
        case '!': case '&':
        case '|': case '"':
            return true;
        default:
            return false;
    }
}

LexerOutput Lexer::analyze(const std::string& input, SymbolTable& symTable) {
>>>>>>> main
    std::vector<Token> tokens;
    std::vector<LexerError> errors;

    const auto& chars = input;
    int len = static_cast<int>(chars.size());
<<<<<<< HEAD

=======
>>>>>>> main
    int i = 0;
    int line = 1;
    int col = 1;

    while (i < len) {
        char c = chars[i];

<<<<<<< HEAD
        // 1. Manejo de saltos de línea y espacios en blanco
=======
>>>>>>> main
        if (c == '\n') {
            line++;
            col = 1;
            i++;
            continue;
<<<<<<< HEAD
        } else if (c == '\r') {
            // Ignorar retorno de carro (para compatibilidad Windows \r\n)
            i++;
            continue;
        } else if (c == ' ' || c == '\t') {
=======
        } else if (c == '\r' || c == ' ' || c == '\t') {
>>>>>>> main
            col++;
            i++;
            continue;
        }

<<<<<<< HEAD
        // 2. Reconocimiento de Identificadores (ID) y Palabras Reservadas (KW_*)
=======
>>>>>>> main
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            int start_col = col;
            std::string ident;

            while (i < len && (std::isalnum(static_cast<unsigned char>(chars[i])) || chars[i] == '_')) {
                ident += chars[i];
                i++;
                col++;
            }

<<<<<<< HEAD
            // Discriminación: Palabra Reservada vs Identificador
            auto kw = check_keyword(ident);
            std::string token_type = kw.has_value() ? kw.value() : "ID";

            tokens.push_back(Token{token_type, ident, line, start_col});
            continue;
        }

        // 3. Reconocimiento de Cadenas de Texto / Strings ("..." o '...')
        if (c == '"' || c == '\'') {
            char quote = c;
            int start_col = col;
            int start_line = line;
            std::string str_val;
            str_val += quote;
            i++;
            col++;

=======
            auto kw = check_keyword(ident);
            if (kw.has_value()) {
                tokens.push_back(Token{kw.value(), ident, -1, line, start_col});
            } else {
                int attr = symTable.insertOrGet(ident);
                tokens.push_back(Token{TokenType::ID, ident, attr, line, start_col});
            }
            continue;
        }

        bool is_open_quote = (c == '"');
        bool is_utf8_quote = (!is_open_quote && static_cast<unsigned char>(c) == 0xe2 && i + 2 < len &&
                              static_cast<unsigned char>(chars[i+1]) == 0x80 &&
                              (static_cast<unsigned char>(chars[i+2]) == 0x9c || static_cast<unsigned char>(chars[i+2]) == 0x9d));

        if (is_open_quote || is_utf8_quote) {
            int start_col = col;
            int start_line = line;
            std::string str_val = "\"";
            if (is_open_quote) {
                i++;
                col++;
            } else {
                i += 3;
                col++;
            }
>>>>>>> main
            bool closed = false;

            while (i < len) {
                char current = chars[i];
<<<<<<< HEAD

                // Manejo de caracteres de escape (\", \n, \t, \\, etc.)
                if (current == '\\') {
                    str_val += '\\';
                    i++;
                    col++;
                    if (i < len) {
                        str_val += chars[i];
                        i++;
                        col++;
                    }
                    continue;
                }

                // Salto de línea inesperado dentro de la cadena
                if (current == '\n') {
                    break;
                }

                // Cierre de la cadena
                if (current == quote) {
                    str_val += quote;
                    i++;
=======
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
                if (static_cast<unsigned char>(current) == 0xe2 && i + 2 < len &&
                    static_cast<unsigned char>(chars[i+1]) == 0x80 &&
                    (static_cast<unsigned char>(chars[i+2]) == 0x9c || static_cast<unsigned char>(chars[i+2]) == 0x9d)) {
                    str_val += '"';
                    i += 3;
>>>>>>> main
                    col++;
                    closed = true;
                    break;
                }
<<<<<<< HEAD

                str_val += current;
                i++;
                col++;
            }

            if (closed) {
                tokens.push_back(Token{"STRING", str_val, start_line, start_col});
            } else {
                std::string msg = "Cadena de texto no cerrada con comilla (";
                msg += quote;
                msg += ")";
                errors.push_back(LexerError{start_line, start_col, str_val, msg});
            }

            continue;
        }

        // 4. Reconocimiento de NUM_INT y NUM_DEC
        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start_col = col;
            std::string num_str;
            bool is_decimal = false;

            // Leer parte entera
            while (i < len && std::isdigit(static_cast<unsigned char>(chars[i]))) {
                num_str += chars[i];
                i++;
                col++;
            }

            // Verificar si hay punto decimal seguido de AL MENOS un dígito
            if (i < len && chars[i] == '.') {
                if (i + 1 < len && std::isdigit(static_cast<unsigned char>(chars[i + 1]))) {
                    is_decimal = true;
                    num_str += '.'; // Agregar el punto
                    i++;
                    col++;

                    // Leer parte decimal
                    while (i < len && std::isdigit(static_cast<unsigned char>(chars[i]))) {
                        num_str += chars[i];
                        i++;
                        col++;
                    }
                }
            }

            std::string token_type = is_decimal ? "NUM_DEC" : "NUM_INT";
            tokens.push_back(Token{token_type, num_str, line, start_col});
            continue;
        }

        // 5. Todo carácter no reconocido se reporta como Error Léxico
        std::string msg = "Símbolo no reconocido: '";
        msg += c;
        msg += "'";
        errors.push_back(LexerError{line, col, std::string(1, c), msg});

        i++;
        col++;
    }

    return LexerOutput{tokens, errors};
=======
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

        // Manejo de números
        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start_col = col;
            std::string num_str;
            int dot_count = 0;
            bool has_invalid_chars = false;

            while (i < len) {
                char current = chars[i];

                if (std::isdigit(static_cast<unsigned char>(current))) {
                    num_str += current;
                    i++; 
                    col++;
                } else if (current == '.') {
                    dot_count++;
                    num_str += current;
                    i++; 
                    col++;
                } else if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
                    has_invalid_chars = true;
                    num_str += current;
                    i++;
                    col++;
                } else {
                    break;
                }
            }

            // Validación de lexema numérico consolidado
            if (dot_count > 1 || has_invalid_chars || num_str.back() == '.') {
                errors.push_back(LexerError{line, start_col, num_str, "Numero mal formado"});
            } else if (dot_count == 1) {
                tokens.push_back(Token{TokenType::NUM_DEC, num_str, -1, line, start_col});
            } else {
                tokens.push_back(Token{TokenType::NUM_INT, num_str, -1, line, start_col});
            }
            continue;
        }

        // Comentarios de una línea (//.*\n) y Operador aritmético división (/)
        if (c == '/') {
            if (i + 1 < len && chars[i + 1] == '/') {
                i += 2;
                col += 2;
                while (i < len && chars[i] != '\n') {
                    i++;
                    col++;
                }
                if (i < len && chars[i] == '\n') {
                    line++;
                    col = 1;
                    i++;
                }
                continue;
            } else {
                tokens.push_back(Token{TokenType::DIV, "/", -1, line, col});
                i++;
                col++;
                continue;
            }
        }

        // Operador de asignación (=) y Operador relacional de igualdad (==)
        if (c == '=') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '=') {
                tokens.push_back(Token{TokenType::COMP, "==", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                tokens.push_back(Token{TokenType::ASSIGN, "=", -1, line, start_col});
                i++;
                col++;
            }
            continue;
        }

        // Operador lógico negación (!) y Operador relacional diferente (!=)
        if (c == '!') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '=') {
                tokens.push_back(Token{TokenType::COMP, "!=", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                tokens.push_back(Token{TokenType::NOT, "!", -1, line, start_col});
                i++;
                col++;
            }
            continue;
        }

        // Operadores relacionales (<, <=)
        if (c == '<') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '=') {
                tokens.push_back(Token{TokenType::COMP, "<=", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                tokens.push_back(Token{TokenType::COMP, "<", -1, line, start_col});
                i++;
                col++;
            }
            continue;
        }

        // Operadores relacionales (>, >=)
        if (c == '>') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '=') {
                tokens.push_back(Token{TokenType::COMP, ">=", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                tokens.push_back(Token{TokenType::COMP, ">", -1, line, start_col});
                i++;
                col++;
            }
            continue;
        }

        // Operador lógico AND (&&)
        if (c == '&') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '&') {
                tokens.push_back(Token{TokenType::AND, "&&", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                errors.push_back(LexerError{line, start_col, "&", "Simbolo no reconocido"});
                i++;
                col++;
            }
            continue;
        }

        // Operador lógico OR (||)
        if (c == '|') {
            int start_col = col;
            if (i + 1 < len && chars[i + 1] == '|') {
                tokens.push_back(Token{TokenType::OR, "||", -1, line, start_col});
                i += 2;
                col += 2;
            } else {
                errors.push_back(LexerError{line, start_col, "|", "Simbolo no reconocido"});
                i++;
                col++;
            }
            continue;
        }

        // Operadores aritméticos (+, -, *, %)
        if (c == '+') {
            tokens.push_back(Token{TokenType::PLUS, "+", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '-') {
            tokens.push_back(Token{TokenType::MINUS, "-", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '*') {
            tokens.push_back(Token{TokenType::MULT, "*", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '%') {
            tokens.push_back(Token{TokenType::MOD, "%", -1, line, col});
            i++; col++;
            continue;
        }

        // Símbolos especiales: (, ), [, ], {, }, ,, ;
        if (c == '(') {
            tokens.push_back(Token{TokenType::LPAREN, "(", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == ')') {
            tokens.push_back(Token{TokenType::RPAREN, ")", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '[') {
            tokens.push_back(Token{TokenType::LBRACKET, "[", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == ']') {
            tokens.push_back(Token{TokenType::RBRACKET, "]", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '{') {
            tokens.push_back(Token{TokenType::LBRACE, "{", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == '}') {
            tokens.push_back(Token{TokenType::RBRACE, "}", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == ',') {
            tokens.push_back(Token{TokenType::COMMA, ",", -1, line, col});
            i++; col++;
            continue;
        }
        if (c == ';') {
            tokens.push_back(Token{TokenType::SEMICOLON, ";", -1, line, col});
            i++; col++;
            continue;
        }

        // Captura de símbolos y variables inválidas completas (ej: @variableInvalida, $precio)
        int start_col = col;
        std::string err_lexeme;
        err_lexeme += c;
        i++;
        col++;

        while (i < len) {
            char next = chars[i];
            if (next == ' ' || next == '\t' || next == '\r' || next == '\n' || is_token_boundary(next)) {
                break;
            }
            err_lexeme += next;
            i++;
            col++;
        }

        errors.push_back(LexerError{line, start_col, err_lexeme, "Simbolo no reconocido"});
    }

    return LexerOutput{tokens, errors, symTable.to_json()};
>>>>>>> main
}
