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
    for (std::size_t i = 0; i < errors.size(); ++i) {
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

    // Evita duplicar exactamente el mismo error en la misma posición.
    for (const auto& existing : errors_) {
        if (existing.line == error.line && existing.col == error.col &&
            existing.message == error.message) {
            return;
        }
    }

    if (errors_.size() < 100) {
        errors_.push_back(error);
    }
}

// Recuperación general: avanza hasta un punto seguro sin consumir el '}'
// que cierra el bloque actual.
void Parser::synchronize() {
    int braces = 0;
    int parens = 0;

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON && braces == 0 &&
            parens == 0) {
            return;
        }

        switch (peek().type) {
            case TokenType::LBRACE:
                ++braces;
                advance();
                continue;
            case TokenType::RBRACE:
                if (braces == 0) {
                    return;
                }
                --braces;
                advance();
                continue;
            case TokenType::LPAREN:
                ++parens;
                advance();
                continue;
            case TokenType::RPAREN:
                if (parens > 0) {
                    --parens;
                }
                advance();
                continue;
            default:
                break;
        }

        if (braces == 0 && parens == 0) {
            switch (peek().type) {
                case TokenType::ID:
                case TokenType::KW_INT:
                case TokenType::KW_FLOAT:
                case TokenType::KW_CHAR:
                case TokenType::KW_BOOLEAN:
                case TokenType::KW_IF:
                case TokenType::KW_WHILE:
                case TokenType::KW_RETURN:
                case TokenType::KW_SCANF:
                case TokenType::KW_PRINTLN:
                case TokenType::KW_FUNC:
                    return;
                default:
                    break;
            }
        }

        advance();
    }
}

void Parser::recoverCommand() {
    int braces = 0;
    int parens = 0;

    while (!isAtEnd()) {
        TokenType type = peek().type;
        if (type == TokenType::LBRACE) {
            ++braces;
        } else if (type == TokenType::RBRACE) {
            if (braces == 0) {
                return;
            }
            --braces;
        } else if (type == TokenType::LPAREN) {
            ++parens;
        } else if (type == TokenType::RPAREN) {
            if (parens > 0) {
                --parens;
            }
        } else if (type == TokenType::SEMICOLON && braces == 0 &&
                   parens == 0) {
            advance();
            return;
        }
        advance();
    }
}

void Parser::recoverTopLevel() {
    while (!isAtEnd()) {
        if (isPrincipalStart() || isTypeStart() || check(TokenType::KW_VOID)) {
            return;
        }
        if (check(TokenType::LBRACE)) {
            skipBalancedBlock();
            continue;
        }
        advance();
    }
}

void Parser::skipBalancedBlock() {
    if (!check(TokenType::LBRACE)) {
        return;
    }

    int depth = 0;
    do {
        if (check(TokenType::LBRACE)) {
            ++depth;
        } else if (check(TokenType::RBRACE)) {
            --depth;
        }
        advance();
    } while (depth > 0 && !isAtEnd());
}

void Parser::skipUnsupportedCommand(const std::string& name) {
    report(peek(), "comando implementado en esta entrega",
           "La construcción '" + name +
               "' se implementará en la entrega completa.");
    if (!isAtEnd()) {
        advance();
    }
    synchronize();
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
    std::vector<ASTNodePtr> functions = parseListaFunciones();
    ASTNodePtr mainNode;
    bool mainFound = false;

    if (isPrincipalStart()) {
        mainNode = parsePrincipal();
        mainFound = true;
    } else {
        report(peek(), "main()");
    }

    if (!isAtEnd()) {
        if (mainFound) {
            report(peek(), "fin de archivo");
        }
        while (!isAtEnd()) {
            advance();
        }
    }

    ParseResult result;
    result.executed = true;
    result.accepted = errors_.empty();
    result.errors = errors_;

    if (result.accepted) {
        Token start = tokens_.empty() ? eof_ : tokens_.front();
        auto program = makeNode(ASTKind::Program, start);
        for (const auto& fn : functions) {
            addChild(program, "function", fn);
        }
        if (mainNode) {
            addChild(program, "main", mainNode);
        }
        result.ast = program;
    }

    return result;
}

// LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | ε
std::vector<ASTNodePtr> Parser::parseListaFunciones() {
    std::vector<ASTNodePtr> functions;

    while (!isAtEnd() && !isPrincipalStart()) {
        if (!isTypeStart() && !check(TokenType::KW_VOID)) {
            break;
        }

        // int main(), float main(), etc. no pertenecen a la gramática.
        if (peekNext().type == TokenType::KW_MAIN) {
            report(peekNext(), "main() sin tipo de retorno",
                   "main no lleva tipo de retorno; usa void main() o main().");
            advance(); // consume el tipo
            break;
        }

        std::size_t before = pos_;
        ASTNodePtr function = parseDeclaracionFuncion();
        if (function) {
            functions.push_back(function);
        }
        if (pos_ == before) {
            advance();
        }
    }

    return functions;
}

// DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE
ASTNodePtr Parser::parseDeclaracionFuncion() {
    ASTNodePtr returnType = parseTipoRetorno();

    Token nameToken;
    if (check(TokenType::ID)) {
        nameToken = advance();
    } else {
        expect(TokenType::ID, "identificador de función");
        recoverTopLevel();
        return nullptr;
    }

    if (!expect(TokenType::LPAREN, "'('")) {
        recoverTopLevel();
        return nullptr;
    }

    std::vector<ASTNodePtr> params = parseParametros();
    expect(TokenType::RPAREN, "')'");
    ASTNodePtr body = parseBloque();

    auto function = makeNode(ASTKind::Function, nameToken, nameToken.lexeme);
    addChild(function, "returnType", returnType);
    for (const auto& param : params) {
        addChild(function, "param", param);
    }
    addChild(function, "body", body);
    return function;
}

// TIPO_RETORNO -> TIPO | void
ASTNodePtr Parser::parseTipoRetorno() {
    if (match(TokenType::KW_VOID)) {
        Token token = previous();
        return makeNode(ASTKind::Type, token, "void");
    }
    return parseTipo();
}

// TIPO -> TIPO_BASE DIMENSION
ASTNodePtr Parser::parseTipo() {
    Token start = peek();
    std::string base = parseTipoBase();
    auto type = makeNode(ASTKind::Type, start, base);
    parseDimension(type);
    return type;
}

// TIPO_BASE -> char | float | int | boolean
std::string Parser::parseTipoBase() {
    switch (peek().type) {
        case TokenType::KW_CHAR:
            advance();
            return "char";
        case TokenType::KW_FLOAT:
            advance();
            return "float";
        case TokenType::KW_INT:
            advance();
            return "int";
        case TokenType::KW_BOOLEAN:
            advance();
            return "boolean";
        default:
            report(peek(), "tipo base (char, float, int o boolean)");
            return "desconocido";
    }
}

// DIMENSION -> [ num_int ] DIMENSION | ε
void Parser::parseDimension(const ASTNodePtr& target, const std::string& role) {
    while (match(TokenType::LBRACKET)) {
        if (check(TokenType::NUM_INT)) {
            Token number = advance();
            addChild(target, role,
                     makeNode(ASTKind::Dimension, number, number.lexeme,
                              "num_int"));
        } else {
            report(peek(), "num_int");
        }
        expect(TokenType::RBRACKET, "']'");
    }
}

// PARAMETROS -> TIPO id LISTA_PARAMETROS | ε
std::vector<ASTNodePtr> Parser::parseParametros() {
    std::vector<ASTNodePtr> params;

    if (check(TokenType::KW_VOID)) {
        // La gramática no contempla void como tipo de parámetro.
        return params;
    }

    if (isTypeStart()) {
        ASTNodePtr type = parseTipo();

        Token nameToken;
        if (check(TokenType::ID)) {
            nameToken = advance();
        } else {
            expect(TokenType::ID, "identificador de parámetro");
            return params;
        }

        auto param = makeNode(ASTKind::Param, nameToken, nameToken.lexeme);
        addChild(param, "type", type);
        // Compatibilidad con parámetros estilo "int datos[10]".
        parseDimension(param);
        params.push_back(param);
        parseListaParametros(params);
    }

    return params;
}

// LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | ε
void Parser::parseListaParametros(std::vector<ASTNodePtr>& params) {
    while (match(TokenType::COMMA)) {
        if (!isTypeStart()) {
            report(peek(), "tipo de parámetro");
            return;
        }

        ASTNodePtr type = parseTipo();

        if (!check(TokenType::ID)) {
            expect(TokenType::ID, "identificador de parámetro");
            return;
        }
        Token nameToken = advance();

        auto param = makeNode(ASTKind::Param, nameToken, nameToken.lexeme);
        addChild(param, "type", type);
        parseDimension(param);
        params.push_back(param);
    }
}

// PRINCIPAL -> [void] main ( ) BLOQUE
ASTNodePtr Parser::parsePrincipal() {
    Token start = peek();
    match(TokenType::KW_VOID);
    expect(TokenType::KW_MAIN, "main");
    Token nameToken = previous();
    expect(TokenType::LPAREN, "'('");
    expect(TokenType::RPAREN, "')'");
    ASTNodePtr body = parseBloque();

    auto mainNode = makeNode(ASTKind::Main, nameToken, "main");
    addChild(mainNode, "body", body);
    (void)start;
    return mainNode;
}

// BLOQUE -> { LISTA_VARIABLES COMANDOS }
ASTNodePtr Parser::parseBloque() {
    Token start = peek();
    if (!expect(TokenType::LBRACE, "'{'")) {
        recoverTopLevel();
        return nullptr;
    }

    auto block = makeNode(ASTKind::Block, start);
    std::vector<ASTNodePtr> variables = parseListaVariables();
    for (const auto& variable : variables) {
        addChild(block, "declaration", variable);
    }

    std::vector<ASTNodePtr> commands = parseComandos();
    for (const auto& command : commands) {
        addChild(block, "command", command);
    }

    expect(TokenType::RBRACE, "'}'");
    return block;
}

// LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | ε
std::vector<ASTNodePtr> Parser::parseListaVariables() {
    std::vector<ASTNodePtr> variables;

    while (isTypeStart()) {
        std::size_t before = pos_;
        Token start = peek();
        ASTNodePtr type = parseTipo();

        if (!check(TokenType::ID)) {
            expect(TokenType::ID, "identificador");
            recoverCommand();
            if (pos_ == before) {
                advance();
            }
            continue;
        }

        Token firstToken = advance();
        auto firstDeclarator =
            makeNode(ASTKind::Declarator, firstToken, firstToken.lexeme);
        // Compatibilidad con "int datos[10]".
        parseDimension(firstDeclarator);

        auto varDecl = makeNode(ASTKind::VarDecl, start,
                                type ? type->value : "");
        addChild(varDecl, "type", type);
        addChild(varDecl, "declarator", firstDeclarator);
        parseListaId(varDecl);
        variables.push_back(varDecl);

        if (!expect(TokenType::SEMICOLON, "';'")) {
            recoverCommand();
        }
    }

    return variables;
}

// LISTA_ID -> , id LISTA_ID | ε
void Parser::parseListaId(const ASTNodePtr& varDecl) {
    while (match(TokenType::COMMA)) {
        if (!check(TokenType::ID)) {
            expect(TokenType::ID, "identificador");
            return;
        }
        Token token = advance();
        addChild(varDecl, "declarator",
                 makeNode(ASTKind::Declarator, token, token.lexeme));
    }
}

// COMANDOS -> COMANDO COMANDOS | ε
std::vector<ASTNodePtr> Parser::parseComandos() {
    std::vector<ASTNodePtr> commands;

    while (!isAtEnd() && !check(TokenType::RBRACE)) {
        std::size_t before = pos_;
        ASTNodePtr command = parseComando();
        if (command) {
            commands.push_back(command);
        }
        if (pos_ == before) {
            advance();
        }
    }

    return commands;
}

// COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION |
//            SELECCION | MIENTRAS | RETORNO
ASTNodePtr Parser::parseComando() {
    if (check(TokenType::ID)) {
        return parseAsignacion();
    }

    switch (peek().type) {
        case TokenType::KW_SCANF:
            return parseLectura();
        case TokenType::KW_PRINTLN:
            return parseEscritura();
        case TokenType::KW_IF:
            return parseSeleccion();
        case TokenType::KW_WHILE:
            return parseMientras();
        case TokenType::KW_RETURN:
            return parseRetorno();
        case TokenType::KW_FUNC:
            return parseFuncionComoComando();
        case TokenType::KW_FOR:
            report(peek(), "comando válido",
                   "'for' no forma parte de la gramática de LP.");
            advance();
            if (check(TokenType::LBRACE)) {
                skipBalancedBlock();
            } else {
                synchronize();
            }
            return nullptr;
        case TokenType::KW_ELSE:
            report(peek(), "if",
                   "Se encontró 'else' sin un 'if' que lo preceda.");
            advance();
            recoverCommand();
            return nullptr;
        default:
            report(peek(), "comando válido");
            recoverCommand();
            return nullptr;
    }
}

// ASIGNACION -> id [ DIMENSION2 ] = COMPLEMENTO ;
ASTNodePtr Parser::parseAsignacion() {
    if (!check(TokenType::ID)) {
        expect(TokenType::ID, "identificador");
        recoverCommand();
        return nullptr;
    }

    Token idToken = advance();
    ASTNodePtr target =
        makeNode(ASTKind::Identifier, idToken, idToken.lexeme);
    // Compatibilidad con arreglos en el lado izquierdo: datos[i] = ...
    target = parseDimension2(target);

    if (!expect(TokenType::ASSIGN, "'='")) {
        recoverCommand();
        return target;
    }

    ASTNodePtr value = parseComplemento();
    expect(TokenType::SEMICOLON, "';'");

    auto assign = makeNode(ASTKind::Assign, idToken);
    addChild(assign, "target", target);
    addChild(assign, "value", value);
    return assign;
}

// COMPLEMENTO -> EXPRESION | FUNCION
ASTNodePtr Parser::parseComplemento() {
    if (check(TokenType::KW_FUNC)) {
        return parseFuncion();
    }
    return parseExpresion();
}

// FUNCION -> func id ( ARGUMENTOS )
ASTNodePtr Parser::parseFuncion() {
    expect(TokenType::KW_FUNC, "func");

    Token nameToken;
    if (check(TokenType::ID)) {
        nameToken = advance();
    } else {
        expect(TokenType::ID, "identificador de función");
        recoverCommand();
        return nullptr;
    }

    if (!expect(TokenType::LPAREN, "'('")) {
        recoverCommand();
        return nullptr;
    }

    std::vector<ASTNodePtr> args = parseArgumentos();
    expect(TokenType::RPAREN, "')'");

    auto call = makeNode(ASTKind::Call, nameToken, nameToken.lexeme);
    for (const auto& arg : args) {
        addChild(call, "argument", arg);
    }
    return call;
}

// FUNCION como comando. La gramática no exige ';', pero se acepta de forma
// opcional para compatibilidad con la notación habitual.
ASTNodePtr Parser::parseFuncionComoComando() {
    ASTNodePtr call = parseFuncion();
    match(TokenType::SEMICOLON);
    return call;
}

// ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | ε
std::vector<ASTNodePtr> Parser::parseArgumentos() {
    std::vector<ASTNodePtr> args;

    if (check(TokenType::RPAREN) || isAtEnd()) {
        return args;
    }

    ASTNodePtr first = parseExpresion();
    if (first) {
        args.push_back(first);
    }

    while (match(TokenType::COMMA)) {
        ASTNodePtr arg = parseExpresion();
        if (arg) {
            args.push_back(arg);
        } else {
            break;
        }
    }

    return args;
}

// LECTURA -> scanf ( TERMINO_LECTURA NUEVO_TERMINO_LECTURA ) ;
ASTNodePtr Parser::parseLectura() {
    Token start = peek();
    expect(TokenType::KW_SCANF, "scanf");
    if (!expect(TokenType::LPAREN, "'('")) {
        recoverCommand();
        return nullptr;
    }

    auto read = makeNode(ASTKind::Read, start, "scanf");
    if (check(TokenType::RPAREN)) {
        report(peek(), "término de lectura",
               "scanf requiere al menos un identificador.");
    } else {
        ASTNodePtr term = parseTerminoLectura();
        addChild(read, "term", term);
        while (match(TokenType::COMMA)) {
            ASTNodePtr next = parseTerminoLectura();
            if (!next) {
                break;
            }
            addChild(read, "term", next);
        }
    }

    if (!check(TokenType::RPAREN)) {
        report(peek(), "')'");
        recoverCommand();
        return read;
    }
    advance();
    if (!expect(TokenType::SEMICOLON, "';'")) {
        recoverCommand();
    }
    return read;
}

// TERMINO_LECTURA -> id DIMENSION2
ASTNodePtr Parser::parseTerminoLectura() {
    if (!check(TokenType::ID)) {
        expect(TokenType::ID, "identificador de lectura");
        return nullptr;
    }

    Token token = advance();
    ASTNodePtr identifier =
        makeNode(ASTKind::Identifier, token, token.lexeme);
    return parseDimension2(identifier);
}

// ESCRITURA -> println ( TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA ) ;
ASTNodePtr Parser::parseEscritura() {
    Token start = peek();
    expect(TokenType::KW_PRINTLN, "println");
    if (!expect(TokenType::LPAREN, "'('")) {
        recoverCommand();
        return nullptr;
    }

    auto write = makeNode(ASTKind::Write, start, "println");
    if (check(TokenType::RPAREN)) {
        report(peek(), "término de escritura",
               "println requiere al menos un término.");
    } else {
        ASTNodePtr term = parseTerminoEscritura();
        addChild(write, "term", term);
        while (match(TokenType::COMMA)) {
            ASTNodePtr next = parseTerminoEscritura();
            if (!next) {
                break;
            }
            addChild(write, "term", next);
        }
    }

    if (!check(TokenType::RPAREN)) {
        report(peek(), "')'");
        recoverCommand();
        return write;
    }
    advance();
    if (!expect(TokenType::SEMICOLON, "';'")) {
        recoverCommand();
    }
    return write;
}

// TERMINO_ESCRITURA -> id DIMENSION2 | CONSTANTE | texto
ASTNodePtr Parser::parseTerminoEscritura() {
    if (check(TokenType::ID)) {
        Token token = advance();
        ASTNodePtr identifier =
            makeNode(ASTKind::Identifier, token, token.lexeme);
        return parseDimension2(identifier);
    }

    if (check(TokenType::NUM_INT) || check(TokenType::NUM_DEC)) {
        Token token = advance();
        return makeNode(ASTKind::Literal, token, token.lexeme,
                        token.type == TokenType::NUM_INT ? "num_int"
                                                          : "num_dec");
    }

    if (check(TokenType::TEXTO)) {
        Token token = advance();
        return makeNode(ASTKind::Literal, token, token.lexeme, "texto");
    }

    report(peek(), "término de escritura (identificador, constante o texto)");
    return nullptr;
}

// SELECCION -> if ( EXPRESION ) BLOQUE SINO
ASTNodePtr Parser::parseSeleccion() {
    Token start = peek();
    expect(TokenType::KW_IF, "if");
    if (!expect(TokenType::LPAREN, "'('")) {
        recoverCommand();
        return nullptr;
    }

    ASTNodePtr condition = parseExpresion();
    expect(TokenType::RPAREN, "')'");
    ASTNodePtr thenBlock = parseBloque();

    auto selection = makeNode(ASTKind::If, start);
    addChild(selection, "condition", condition);
    addChild(selection, "then", thenBlock);

    if (match(TokenType::KW_ELSE)) {
        ASTNodePtr elseBlock = parseBloque();
        addChild(selection, "else", elseBlock);
    }

    return selection;
}

// MIENTRAS -> while ( EXPRESION ) BLOQUE
ASTNodePtr Parser::parseMientras() {
    Token start = peek();
    expect(TokenType::KW_WHILE, "while");
    if (!expect(TokenType::LPAREN, "'('")) {
        recoverCommand();
        return nullptr;
    }

    ASTNodePtr condition = parseExpresion();
    expect(TokenType::RPAREN, "')'");
    ASTNodePtr body = parseBloque();

    auto loop = makeNode(ASTKind::While, start);
    addChild(loop, "condition", condition);
    addChild(loop, "body", body);
    return loop;
}

// RETORNO -> return EXPRESION ;
ASTNodePtr Parser::parseRetorno() {
    Token start = peek();
    expect(TokenType::KW_RETURN, "return");
    ASTNodePtr value = parseExpresion();
    expect(TokenType::SEMICOLON, "';'");

    auto ret = makeNode(ASTKind::Return, start);
    addChild(ret, "value", value);
    return ret;
}

// EXPRESION -> EXPRESION_O
ASTNodePtr Parser::parseExpresion() {
    return parseExpresionO();
}

// EXPRESION_O -> EXPRESION_Y ( || EXPRESION_Y )*
ASTNodePtr Parser::parseExpresionO() {
    ASTNodePtr left = parseExpresionY();
    if (!left) {
        return nullptr;
    }

    while (check(TokenType::OR)) {
        Token op = advance();
        ASTNodePtr right = parseExpresionY();
        auto binary = makeNode(ASTKind::Binary, op, op.lexeme);
        addChild(binary, "left", left);
        addChild(binary, "right", right);
        left = binary;
    }

    return left;
}

// EXPRESION_Y -> EXPRESION_RELACIONAL ( && EXPRESION_RELACIONAL )*
ASTNodePtr Parser::parseExpresionY() {
    ASTNodePtr left = parseExpresionRelacional();
    if (!left) {
        return nullptr;
    }

    while (check(TokenType::AND)) {
        Token op = advance();
        ASTNodePtr right = parseExpresionRelacional();
        auto binary = makeNode(ASTKind::Binary, op, op.lexeme);
        addChild(binary, "left", left);
        addChild(binary, "right", right);
        left = binary;
    }

    return left;
}

// EXPRESION_RELACIONAL -> EXPRESION_ADITIVA [ comp EXPRESION_ADITIVA ]
ASTNodePtr Parser::parseExpresionRelacional() {
    ASTNodePtr left = parseExpresionAditiva();
    if (!left) {
        return nullptr;
    }

    if (check(TokenType::COMP)) {
        Token op = advance();
        ASTNodePtr right = parseExpresionAditiva();
        auto binary = makeNode(ASTKind::Binary, op, op.lexeme);
        addChild(binary, "left", left);
        addChild(binary, "right", right);
        left = binary;
    }

    return left;
}

// EXPRESION_ADITIVA -> EXPRESION_MULTIPLICATIVA ( (+|-) ... )*
ASTNodePtr Parser::parseExpresionAditiva() {
    ASTNodePtr left = parseExpresionMultiplicativa();
    if (!left) {
        return nullptr;
    }

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        ASTNodePtr right = parseExpresionMultiplicativa();
        auto binary = makeNode(ASTKind::Binary, op, op.lexeme);
        addChild(binary, "left", left);
        addChild(binary, "right", right);
        left = binary;
    }

    return left;
}

// EXPRESION_MULTIPLICATIVA -> FACTOR ( (*|/|%) FACTOR )*
ASTNodePtr Parser::parseExpresionMultiplicativa() {
    ASTNodePtr left = parseFactor();
    if (!left) {
        return nullptr;
    }

    while (check(TokenType::MULT) || check(TokenType::DIV) ||
           check(TokenType::MOD)) {
        Token op = advance();
        ASTNodePtr right = parseFactor();
        auto binary = makeNode(ASTKind::Binary, op, op.lexeme);
        addChild(binary, "left", left);
        addChild(binary, "right", right);
        left = binary;
    }

    return left;
}

// FACTOR -> SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION )
ASTNodePtr Parser::parseFactor() {
    if (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        ASTNodePtr operand = parseTermino();
        auto unary = makeNode(ASTKind::Unary, op, op.lexeme);
        addChild(unary, "operand", operand);
        return unary;
    }

    if (match(TokenType::TEXTO)) {
        Token token = previous();
        return makeNode(ASTKind::Literal, token, token.lexeme, "texto");
    }

    if (match(TokenType::NOT)) {
        Token op = previous();
        ASTNodePtr operand = parseFactor();
        auto unary = makeNode(ASTKind::Unary, op, op.lexeme);
        addChild(unary, "operand", operand);
        return unary;
    }

    if (match(TokenType::LPAREN)) {
        ASTNodePtr expression = parseExpresion();
        expect(TokenType::RPAREN, "')'");
        return expression;
    }

    if (check(TokenType::ID) || check(TokenType::NUM_INT) ||
        check(TokenType::NUM_DEC)) {
        return parseTermino();
    }

    report(peek(), "expresión (identificador, constante, texto, signo, '!' o '(')");
    return nullptr;
}

// TERMINO -> id DIMENSION2 | CONSTANTE
ASTNodePtr Parser::parseTermino() {
    if (check(TokenType::ID)) {
        Token token = advance();
        ASTNodePtr identifier =
            makeNode(ASTKind::Identifier, token, token.lexeme);
        return parseDimension2(identifier);
    }

    if (check(TokenType::NUM_INT) || check(TokenType::NUM_DEC)) {
        Token token = advance();
        return makeNode(ASTKind::Literal, token, token.lexeme,
                        token.type == TokenType::NUM_INT ? "num_int"
                                                          : "num_dec");
    }

    report(peek(), "identificador o constante");
    return nullptr;
}

// DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | ε
ASTNodePtr Parser::parseDimension2(ASTNodePtr base) {
    while (match(TokenType::LBRACKET)) {
        Token open = previous();
        ASTNodePtr index = parseExpresionAditiva();
        expect(TokenType::RBRACKET, "']'");

        auto access = makeNode(ASTKind::Index, open);
        addChild(access, "base", base);
        addChild(access, "index", index);
        base = access;
    }

    return base;
}
