#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Token.h"

#include <cstddef>
#include <string>
#include <vector>

// Error sintáctico con la ubicación y los tokens involucrados.
struct SyntaxError {
    int line = 0;
    int col = 0;
    std::string found;
    std::string expected;
    std::string message;

    std::string to_json() const;
};

// Resultado del análisis sintáctico.
// executed indica si el parser llegó a ejecutarse; si el lexer produjo
// errores, el análisis sintáctico se omite para evitar errores en cascada.
// ast solo se construye cuando el programa es aceptado.
struct ParseResult {
    bool executed = true;
    bool accepted = false;
    std::vector<SyntaxError> errors;
    ASTNodePtr ast;

    std::string to_json() const;
};

// Analizador sintáctico por descenso recursivo para la gramática de LP.
// Cada método privado corresponde a un no terminal de la gramática.
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // Punto de entrada: PROGRAMA -> LISTA_FUNCIONES PRINCIPAL
    ParseResult parseProgram();

    static ParseResult parse(const std::vector<Token>& tokens);

private:
    const std::vector<Token>& tokens_;
    std::size_t pos_ = 0;
    std::vector<SyntaxError> errors_;
    Token eof_;

    // Utilidades del flujo de tokens.
    const Token& peek() const;
    const Token& peekNext() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    bool expect(TokenType type, const std::string& expected);

    // Manejo de errores y recuperación.
    void report(const Token& token, const std::string& expected,
                const std::string& customMessage = "");
    void synchronize();
    void recoverCommand();
    void recoverTopLevel();
    void skipBalancedBlock();
    void skipUnsupportedCommand(const std::string& name);

    // No terminales de la gramática.
    std::vector<ASTNodePtr> parseListaFunciones();
    ASTNodePtr parseDeclaracionFuncion();
    ASTNodePtr parseTipoRetorno();
    ASTNodePtr parseTipo();
    std::string parseTipoBase();
    void parseDimension(const ASTNodePtr& target,
                        const std::string& role = "dimension");
    std::vector<ASTNodePtr> parseParametros();
    void parseListaParametros(std::vector<ASTNodePtr>& params);
    ASTNodePtr parsePrincipal();
    ASTNodePtr parseBloque();
    std::vector<ASTNodePtr> parseListaVariables();
    void parseListaId(const ASTNodePtr& varDecl);
    std::vector<ASTNodePtr> parseComandos();
    ASTNodePtr parseComando();
    ASTNodePtr parseAsignacion();
    ASTNodePtr parseComplemento();
    ASTNodePtr parseFuncion();
    ASTNodePtr parseFuncionComoComando();
    std::vector<ASTNodePtr> parseArgumentos();

    // Entrada, salida, control y retorno.
    ASTNodePtr parseLectura();
    ASTNodePtr parseEscritura();
    ASTNodePtr parseTerminoLectura();
    ASTNodePtr parseTerminoEscritura();
    ASTNodePtr parseSeleccion();
    ASTNodePtr parseMientras();
    ASTNodePtr parseRetorno();

    // Expresiones, en orden de precedencia.
    ASTNodePtr parseExpresion();
    ASTNodePtr parseExpresionO();
    ASTNodePtr parseExpresionY();
    ASTNodePtr parseExpresionRelacional();
    ASTNodePtr parseExpresionAditiva();
    ASTNodePtr parseExpresionMultiplicativa();
    ASTNodePtr parseFactor();
    ASTNodePtr parseTermino();
    ASTNodePtr parseDimension2(ASTNodePtr base);

    // Auxiliares de decisión.
    bool isTypeStart() const;
    bool isPrincipalStart() const;
    std::string tokenDescription(const Token& token) const;
};

#endif // PARSER_H
