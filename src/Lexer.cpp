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

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return it->second;
    }
    return std::nullopt;
}

/// Ejecuta el análisis léxico sobre el texto de entrada.
LexerOutput Lexer::analyze(const std::string& input) {
    std::vector<Token> tokens;
    std::vector<LexerError> errors;

    const auto& chars = input;
    int len = static_cast<int>(chars.size());

    int i = 0;
    int line = 1;
    int col = 1;

    while (i < len) {
        char c = chars[i];

        // 1. Manejo de saltos de línea y espacios en blanco
        if (c == '\n') {
            line++;
            col = 1;
            i++;
            continue;
        } else if (c == '\r') {
            // Ignorar retorno de carro (para compatibilidad Windows \r\n)
            i++;
            continue;
        } else if (c == ' ' || c == '\t') {
            col++;
            i++;
            continue;
        }

        // 2. Reconocimiento de Identificadores (ID) y Palabras Reservadas (KW_*)
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            int start_col = col;
            std::string ident;

            while (i < len && (std::isalnum(static_cast<unsigned char>(chars[i])) || chars[i] == '_')) {
                ident += chars[i];
                i++;
                col++;
            }

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

            bool closed = false;

            while (i < len) {
                char current = chars[i];

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
                    col++;
                    closed = true;
                    break;
                }

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
}
