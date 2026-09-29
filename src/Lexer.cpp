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
    };//palabras reservadas 

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) { //busca el lexema del mapa
        return it->second;
    }
    return std::nullopt;// is no lo encuntra no es palabra reservada 
}

static inline bool is_token_boundary(char ch) {
    switch (ch) { //son los caracteres que inician un toker propio
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
    std::vector<Token> tokens;
    std::vector<LexerError> errors; //dos listas vacias  se crean durante el analiiss

    const auto& chars = input; //referencia al texto de entrada
    int len = static_cast<int>(chars.size()); //guarda la longitud del texto de entrada 
    int i = 0;//indice para recorrer el texto de entrada
    int line = 1;//guarda el numero de linea del texto de entrada
    int col = 1;//guarda el numero de columna del texto de entrada

    while (i < len) {//mientras el indice sea menor a la longitud del texto de entrada
        char c = chars[i];//guarda el caracter actual del texto de entrada

        if (c == '\n') {//si el caracter actual es un salto de linea
            line++;//incrementa el numero de linea
            col = 1;//reinicia el numero de columna
            i++;//incrementa el indice
            continue;//continua con el siguiente caracter
        } else if (c == '\r' || c == ' ' || c == '\t') {//si el caracter actual es un retorno de carro, espacio o tabulador
            col++;//incrementa el numero de columna
            i++;//incrementa el indice
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {//si el caracter actual es una letra o un guion bajo
            int start_col = col;//guarda el numero de columna donde inicia el lexema
            std::string ident;//guarda el lexema
            ident += c;//agrega el caracter actual al lexema

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
                if (static_cast<unsigned char>(current) == 0xe2 && i + 2 < len &&
                    static_cast<unsigned char>(chars[i+1]) == 0x80 &&
                    (static_cast<unsigned char>(chars[i+2]) == 0x9c || static_cast<unsigned char>(chars[i+2]) == 0x9d)) {
                    str_val += '"';
                    i += 3;
                    col++;
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

        // Manejo robusto de números (Enteros, Decimales y Mal Formados como 1.2.3 o 1.2a)
        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start_col = col;
            std::string num_str;
            int dot_count = 0;
            bool has_invalid_chars = false;

            while (i < len) {
                char current = chars[i];

                if (std::isdigit(static_cast<unsigned char>(current))) {
                    num_str += current;
                    i++; col++;
                } else if (current == '.') {
                    dot_count++;
                    num_str += current;
                    i++; col++;
                } else if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
                    has_invalid_chars = true;
                    num_str += current;
                    i++; col++;
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
}
