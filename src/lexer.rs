use crate::token::{LexerError, LexerOutput, Token};

pub struct Lexer;

impl Lexer {
    /// Determina si un lexema es una palabra reservada y devuelve su tipo de token.
    pub fn check_keyword(lexeme: &str) -> Option<&'static str> {
        match lexeme {
            // Estructuras de control
            "if" => Some("KW_IF"),
            "else" => Some("KW_ELSE"),
            "while" => Some("KW_WHILE"),
            "for" => Some("KW_FOR"),
            "do" => Some("KW_DO"),
            "return" => Some("KW_RETURN"),
            "break" => Some("KW_BREAK"),
            "continue" => Some("KW_CONTINUE"),
            "switch" => Some("KW_SWITCH"),
            "case" => Some("KW_CASE"),
            "default" => Some("KW_DEFAULT"),

            // Declaraciones y tipos de datos
            "let" => Some("KW_LET"),
            "var" => Some("KW_VAR"),
            "const" => Some("KW_CONST"),
            "fn" => Some("KW_FN"),
            "function" => Some("KW_FUNCTION"),
            "int" => Some("KW_INT"),
            "float" => Some("KW_FLOAT"),
            "double" => Some("KW_DOUBLE"),
            "char" => Some("KW_CHAR"),
            "string" => Some("KW_STRING"),
            "bool" => Some("KW_BOOL"),
            "void" => Some("KW_VOID"),
            "struct" => Some("KW_STRUCT"),
            "class" => Some("KW_CLASS"),

            // Literales booleanos / nulos
            "true" => Some("KW_TRUE"),
            "false" => Some("KW_FALSE"),
            "null" => Some("KW_NULL"),

            // Funciones / identificadores clave comunes
            "print" => Some("KW_PRINT"),
            "println" => Some("KW_PRINTLN"),
            "main" => Some("KW_MAIN"),

            _ => None,
        }
    }

    pub fn analyze(input: &str) -> LexerOutput {
        let mut tokens = Vec::new();
        let mut errors = Vec::new();

        let chars: Vec<char> = input.chars().collect();
        let len = chars.len();

        let mut i = 0;
        let mut line = 1;
        let mut col = 1;

        while i < len {
            let c = chars[i];

            // 1. Manejo de saltos de línea y espacios en blanco
            if c == '\n' {
                line += 1;
                col = 1;
                i += 1;
                continue;
            } else if c.is_whitespace() {
                col += 1;
                i += 1;
                continue;
            }

            // 2. Reconocimiento de Identificadores (ID) y Palabras Reservadas (KW_*)
            if c.is_alphabetic() || c == '_' {
                let start_col = col;
                let mut ident = String::new();

                while i < len && (chars[i].is_alphanumeric() || chars[i] == '_') {
                    ident.push(chars[i]);
                    i += 1;
                    col += 1;
                }

                // Discriminación: Palabra Reservada vs Identificador
                let token_type = match Self::check_keyword(&ident) {
                    Some(kw) => kw.to_string(),
                    None => "ID".to_string(),
                };

                tokens.push(Token {
                    r#type: token_type,
                    lexeme: ident,
                    line,
                    col: start_col,
                });

                continue;
            }

            // 3. Reconocimiento de Cadenas de Texto / Strings ("..." o '...')
            if c == '"' || c == '\'' {
                let quote = c;
                let start_col = col;
                let start_line = line;
                let mut str_val = String::new();
                str_val.push(quote);
                i += 1;
                col += 1;

                let mut closed = false;

                while i < len {
                    let current = chars[i];

                    // Manejo de caracteres de escape (\", \n, \t, \\, etc.)
                    if current == '\\' {
                        str_val.push('\\');
                        i += 1;
                        col += 1;
                        if i < len {
                            str_val.push(chars[i]);
                            i += 1;
                            col += 1;
                        }
                        continue;
                    }

                    // Salto de línea inesperado dentro de la cadena
                    if current == '\n' {
                        break;
                    }

                    // Cierre de la cadena
                    if current == quote {
                        str_val.push(quote);
                        i += 1;
                        col += 1;
                        closed = true;
                        break;
                    }

                    str_val.push(current);
                    i += 1;
                    col += 1;
                }

                if closed {
                    tokens.push(Token {
                        r#type: "STRING".to_string(),
                        lexeme: str_val,
                        line: start_line,
                        col: start_col,
                    });
                } else {
                    errors.push(LexerError {
                        line: start_line,
                        col: start_col,
                        lexeme: str_val,
                        message: format!("Cadena de texto no cerrada con comilla ({})", quote),
                    });
                }

                continue;
            }

            // 4. Reconocimiento de NUM_INT y NUM_DEC
            if c.is_ascii_digit() {
                let start_col = col;
                let mut num_str = String::new();
                let mut is_decimal = false;

                // Leer parte entera
                while i < len && chars[i].is_ascii_digit() {
                    num_str.push(chars[i]);
                    i += 1;
                    col += 1;
                }

                // Verificar si hay punto decimal seguido de AL MENOS un dígito
                if i < len && chars[i] == '.' {
                    if i + 1 < len && chars[i + 1].is_ascii_digit() {
                        is_decimal = true;
                        num_str.push('.'); // Agregar el punto
                        i += 1;
                        col += 1;

                        // Leer parte decimal
                        while i < len && chars[i].is_ascii_digit() {
                            num_str.push(chars[i]);
                            i += 1;
                            col += 1;
                        }
                    }
                }

                let token_type = if is_decimal { "NUM_DEC" } else { "NUM_INT" };

                tokens.push(Token {
                    r#type: token_type.to_string(),
                    lexeme: num_str,
                    line,
                    col: start_col,
                });

                continue;
            }

            // 5. Todo carácter no reconocido se reporta como Error Léxico
            errors.push(LexerError {
                line,
                col,
                lexeme: c.to_string(),
                message: format!("Símbolo no reconocido: '{}'", c),
            });

            i += 1;
            col += 1;
        }

        LexerOutput { tokens, errors }
    }
}
