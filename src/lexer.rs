use crate::token::{LexerError, LexerOutput, Token};

pub struct Lexer;

impl Lexer {
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

            // 2. Reconocimiento de NUM_INT y NUM_DEC
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

            // 3. Todo lo que no sea espacio ni número en esta Fase 1 es un Error Léxico
            errors.push(LexerError {
                line,
                col,
                lexeme: c.to_string(),
                message: "Símbolo no reconocido en la Fase 1".to_string(),
            });

            i += 1;
            col += 1;
        }

        LexerOutput { tokens, errors }
    }
}
