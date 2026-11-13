#include "Parser.h"

#include <sstream>

namespace {

std::string escape_json(const std::string& s) {
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

} // namespace

std::string SyntaxError::to_json() const {
    std::ostringstream oss;
    oss << "{\"line\":" << line
        << ",\"col\":" << col
        << ",\"found\":\"" << escape_json(found)
        << "\",\"expected\":\"" << escape_json(expected)
        << "\",\"message\":\"" << escape_json(message) << "\"}";
    return oss.str();
}

std::string ParseResult::to_json() const {
    std::ostringstream oss;
    oss << "{\"executed\":" << (executed ? "true" : "false")
        << ",\"accepted\":" << (accepted ? "true" : "false")
        << ",\"errors\":[";
    for (size_t i = 0; i < errors.size(); ++i) {
        if (i > 0) oss << ",";
        oss << errors[i].to_json();
    }
    oss << "]}";
    return oss.str();
}

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {
    if (tokens_.empty()) {
        eof_ = Token{TokenType::END_OF_FILE, "EOF", -1, 1, 1};
    } else {
        const Token& last = tokens_.back();
        int eofCol = last.col + static_cast<int>(last.lexeme.size());
        eof_ = Token{TokenType::END_OF_FILE, "EOF", -1, last.line, eofCol};
    }
}

ParseResult Parser::parse(const std::vector<Token>& tokens) {
    Parser parser(tokens);
    return parser.parseProgram();
}

const Token& Parser::peek() const {
    if (pos_ < tokens_.size()) {
        return tokens_[pos_];
    }
    return eof_;
}

const Token& Parser::peekNext() const {
    if (pos_ + 1 < tokens_.size()) {
        return tokens_[pos_ + 1];
    }
    return eof_;
}

const Token& Parser::previous() const {
    if (pos_ > 0 && pos_ <= tokens_.size()) {
        return tokens_[pos_ - 1];
    }
    return eof_;
}

bool Parser::isAtEnd() const {
    return pos_ >= tokens_.size();
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++pos_;
    }
    return previous();
}

bool Parser::expect(TokenType type, const std::string& expected) {
    if (check(type)) {
        advance();
        return true;
    }
    report(peek(), expected);
    return false;
}

std::string Parser::tokenDescription(const Token& token) const {
    if (token.type == TokenType::END_OF_FILE) {
        return "fin de archivo";
    }
    if (!token.lexeme.empty()) {
        return token.lexeme;
    }
    return tokenTypeToString(token.type);
}

void Parser::report(const Token& token, const std::string& expected,
                    const std::string& customMessage) {
    SyntaxError error;
    error.line = token.line;
    error.col = token.col;
    error.found = tokenDescription(token);
    error.expected = expected;
    if (!customMessage.empty()) {
        error.message = customMessage;
    } else {
        error.message = "Se esperaba " + expected + "; se encontró '" +
                        error.found + "'.";
    }
    errors_.push_back(error);
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) {
            return;
        }

        switch (peek().type) {
            case TokenType::RBRACE:
            case TokenType::ID:
            case TokenType::KW_INT:
            case TokenType::KW_FLOAT:
            case TokenType::KW_CHAR:
            case TokenType::KW_BOOLEAN:
            case TokenType::KW_IF:
            case TokenType::KW_WHILE:
            case TokenType::KW_FOR:
            case TokenType::KW_RETURN:
            case TokenType::KW_SCANF:
            case TokenType::KW_PRINTLN:
            case TokenType::KW_FUNC:
                return;
            default:
                advance();
                break;
        }
    }
}

void Parser::skipUnsupportedCommand(const std::string& name) {
    report(peek(), "comando implementado en esta entrega",
           "La construcción '" + name +
               "' se implementará en la entrega completa.");
    size_t before = pos_;
    if (!isAtEnd()) {
        advance();
    }
    synchronize();
    if (pos_ == before && !isAtEnd()) {
        advance();
    }
}

bool Parser::isTypeStart() const {
    switch (peek().type) {
        case TokenType::KW_CHAR:
        case TokenType::KW_FLOAT:
        case TokenType::KW_INT:
        case TokenType::KW_BOOLEAN:
            return true;
        default:
            return false;
    }
}

bool Parser::isPrincipalStart() const {
    if (check(TokenType::KW_MAIN)) {
        return true;
    }
    // Compatibilidad con los ejemplos del enunciado: void main().
    return check(TokenType::KW_VOID) && peekNext().type == TokenType::KW_MAIN;
}

ParseResult Parser::parseProgram() {
    parseListaFunciones();

    if (isPrincipalStart()) {
        parsePrincipal();
    } else {
        report(peek(), "main()");
    }

    if (!isAtEnd()) {
        report(peek(), "fin de archivo");
        while (!isAtEnd()) {
            advance();
        }
    }

    ParseResult result;
    result.executed = true;
    result.accepted = errors_.empty();
    result.errors = errors_;
    return result;
}

// LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | ε
void Parser::parseListaFunciones() {
    while (!isAtEnd() && !isPrincipalStart()) {
        if (check(TokenType::KW_VOID) || isTypeStart()) {
            parseDeclaracionFuncion();
        } else {
            break;
        }
    }
}

// DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE
void Parser::parseDeclaracionFuncion() {
    parseTipoRetorno();

    if (!expect(TokenType::ID, "identificador de función")) {
        synchronize();
        return;
    }
    if (!expect(TokenType::LPAREN, "'('")) {
        synchronize();
        return;
    }

    parseParametros();
    expect(TokenType::RPAREN, "')'");
    parseBloque();
}

// TIPO_RETORNO -> TIPO | void
void Parser::parseTipoRetorno() {
    if (match(TokenType::KW_VOID)) {
        return;
    }
    parseTipo();
}

// TIPO -> TIPO_BASE DIMENSION
void Parser::parseTipo() {
    parseTipoBase();
    parseDimension();
}

// TIPO_BASE -> char | float | int | boolean
void Parser::parseTipoBase() {
    switch (peek().type) {
        case TokenType::KW_CHAR:
        case TokenType::KW_FLOAT:
        case TokenType::KW_INT:
        case TokenType::KW_BOOLEAN:
            advance();
            return;
        default:
            report(peek(), "tipo base (char, float, int o boolean)");
            return;
    }
}

// DIMENSION -> [ num_int ] DIMENSION | ε
void Parser::parseDimension() {
    while (match(TokenType::LBRACKET)) {
        expect(TokenType::NUM_INT, "num_int");
        expect(TokenType::RBRACKET, "']'");
    }
}

// PARAMETROS -> TIPO id LISTA_PARAMETROS | ε
void Parser::parseParametros() {
    if (check(TokenType::KW_VOID)) {
        // La gramática no contempla "void" como tipo de parámetro.
        return;
    }
    if (isTypeStart()) {
        parseTipo();
        expect(TokenType::ID, "identificador de parámetro");
        // Compatibilidad con la forma habitual "int datos[10]" además de
        // la forma de la gramática "int[10] datos".
        parseDimension();
        parseListaParametros();
    }
}

// LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | ε
void Parser::parseListaParametros() {
    while (match(TokenType::COMMA)) {
        parseTipo();
        expect(TokenType::ID, "identificador de parámetro");
        parseDimension();
    }
}

// PRINCIPAL -> [void] main ( ) BLOQUE
// El "void" opcional es una extensión de compatibilidad con los ejemplos
// del documento, que usan "void main()".
void Parser::parsePrincipal() {
    match(TokenType::KW_VOID);
    expect(TokenType::KW_MAIN, "main");
    expect(TokenType::LPAREN, "'('");
    expect(TokenType::RPAREN, "')'");
    parseBloque();
}

// BLOQUE -> { LISTA_VARIABLES COMANDOS }
void Parser::parseBloque() {
    if (!expect(TokenType::LBRACE, "'{'")) {
        return;
    }

    parseListaVariables();
    parseComandos();
    expect(TokenType::RBRACE, "'}'");
}

// LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | ε
void Parser::parseListaVariables() {
    while (isTypeStart()) {
        parseTipo();

        if (!expect(TokenType::ID, "identificador")) {
            synchronize();
            continue;
        }

        // Compatibilidad con "int datos[10]" (forma usada en los ejemplos)
        // además de "int[10] datos" (forma literal de la gramática).
        parseDimension();
        parseListaId();
        expect(TokenType::SEMICOLON, "';'");
    }
}

// LISTA_ID -> , id LISTA_ID | ε
void Parser::parseListaId() {
    while (match(TokenType::COMMA)) {
        if (!expect(TokenType::ID, "identificador")) {
            return;
        }
    }
}

// COMANDOS -> COMANDO COMANDOS | ε
void Parser::parseComandos() {
    while (!isAtEnd() && !check(TokenType::RBRACE)) {
        parseComando();
    }
}

// COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION |
//            SELECCION | MIENTRAS | RETORNO
void Parser::parseComando() {
    if (check(TokenType::ID)) {
        parseAsignacion();
        return;
    }

    switch (peek().type) {
        case TokenType::KW_FUNC:
            skipUnsupportedCommand("llamada a función con 'func'");
            return;
        case TokenType::KW_RETURN:
            skipUnsupportedCommand("return");
            return;
        case TokenType::KW_IF:
            skipUnsupportedCommand("if");
            return;
        case TokenType::KW_WHILE:
            skipUnsupportedCommand("while");
            return;
        case TokenType::KW_SCANF:
            skipUnsupportedCommand("scanf");
            return;
        case TokenType::KW_PRINTLN:
            skipUnsupportedCommand("println");
            return;
        case TokenType::KW_FOR:
            report(peek(), "comando válido",
                   "'for' no forma parte de la gramática de LP.");
            advance();
            synchronize();
            return;
        default:
            report(peek(), "comando válido");
            advance();
            synchronize();
            return;
    }
}

// ASIGNACION -> id = COMPLEMENTO ;
void Parser::parseAsignacion() {
    if (!expect(TokenType::ID, "identificador")) {
        synchronize();
        return;
    }
    if (!expect(TokenType::ASSIGN, "'='")) {
        synchronize();
        return;
    }

    parseComplemento();
    expect(TokenType::SEMICOLON, "';'");
}

// COMPLEMENTO -> EXPRESION | FUNCION
void Parser::parseComplemento() {
    if (check(TokenType::KW_FUNC)) {
        parseFuncion();
    } else {
        parseExpresion();
    }
}

// FUNCION -> func id ( ARGUMENTOS )
void Parser::parseFuncion() {
    expect(TokenType::KW_FUNC, "func");
    expect(TokenType::ID, "identificador de función");
    expect(TokenType::LPAREN, "'('");
    parseArgumentos();
    expect(TokenType::RPAREN, "')'");
}

// ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | ε
void Parser::parseArgumentos() {
    if (check(TokenType::RPAREN) || isAtEnd()) {
        return;
    }

    parseExpresion();
    while (match(TokenType::COMMA)) {
        parseExpresion();
    }
}

// EXPRESION -> EXPRESION_O
void Parser::parseExpresion() {
    parseExpresionO();
}

// EXPRESION_O -> EXPRESION_Y EXPRESION_O2
// EXPRESION_O2 -> || EXPRESION_Y EXPRESION_O2 | ε
void Parser::parseExpresionO() {
    parseExpresionY();
    while (match(TokenType::OR)) {
        parseExpresionY();
    }
}

// EXPRESION_Y -> EXPRESION_RELACIONAL EXPRESION_Y2
// EXPRESION_Y2 -> && EXPRESION_RELACIONAL EXPRESION_Y2 | ε
void Parser::parseExpresionY() {
    parseExpresionRelacional();
    while (match(TokenType::AND)) {
        parseExpresionRelacional();
    }
}

// EXPRESION_RELACIONAL -> EXPRESION_ADITIVA EXPRESION_RELACIONAL2
// EXPRESION_RELACIONAL2 -> comp EXPRESION_ADITIVA | ε
void Parser::parseExpresionRelacional() {
    parseExpresionAditiva();
    if (match(TokenType::COMP)) {
        parseExpresionAditiva();
    }
}

// EXPRESION_ADITIVA -> EXPRESION_MULTIPLICATIVA EXPRESION_ADITIVA2
// EXPRESION_ADITIVA2 -> OPERADOR_ADITIVO EXPRESION_MULTIPLICATIVA ... | ε
void Parser::parseExpresionAditiva() {
    parseExpresionMultiplicativa();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        advance();
        parseExpresionMultiplicativa();
    }
}

// EXPRESION_MULTIPLICATIVA -> FACTOR EXPRESION_MULTIPLICATIVA2
// EXPRESION_MULTIPLICATIVA2 -> OPERADOR_MULTIPLICATIVO FACTOR ... | ε
void Parser::parseExpresionMultiplicativa() {
    parseFactor();
    while (check(TokenType::MULT) || check(TokenType::DIV) ||
           check(TokenType::MOD)) {
        advance();
        parseFactor();
    }
}

// FACTOR -> SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION )
void Parser::parseFactor() {
    if (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        parseTermino();
        return;
    }
    if (match(TokenType::TEXTO)) {
        return;
    }
    if (match(TokenType::NOT)) {
        parseFactor();
        return;
    }
    if (match(TokenType::LPAREN)) {
        parseExpresion();
        expect(TokenType::RPAREN, "')'");
        return;
    }
    if (check(TokenType::ID) || check(TokenType::NUM_INT) ||
        check(TokenType::NUM_DEC)) {
        parseTermino();
        return;
    }

    report(peek(), "expresión (identificador, constante, texto, signo, '!' o '(')");
}

// TERMINO -> id DIMENSION2 | CONSTANTE
void Parser::parseTermino() {
    if (check(TokenType::ID)) {
        advance();
        parseDimension2();
        return;
    }
    if (check(TokenType::NUM_INT) || check(TokenType::NUM_DEC)) {
        advance();
        return;
    }

    report(peek(), "identificador o constante");
}

// DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | ε
void Parser::parseDimension2() {
    while (match(TokenType::LBRACKET)) {
        parseExpresionAditiva();
        expect(TokenType::RBRACKET, "']'");
    }
}
