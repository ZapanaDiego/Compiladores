use serde::Serialize;

#[derive(Serialize, Debug, Clone)]
pub struct Token {
    pub r#type: String,  // "NUM_INT" o "NUM_DEC"
    pub lexeme: String,  // p.ej. "20" o "15.5"
    pub line: usize,
    pub col: usize,
}

#[derive(Serialize, Debug, Clone)]
pub struct LexerError {
    pub line: usize,
    pub col: usize,
    pub lexeme: String,
    pub message: String,
}

#[derive(Serialize, Debug)]
pub struct LexerOutput {
    pub tokens: Vec<Token>,
    pub errors: Vec<LexerError>,
}
