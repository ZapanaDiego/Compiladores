#ifndef PARSER_H
#define PARSER_H

#include "Token.h"

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
struct ParseResult {
    bool executed = true;
    bool accepted = false;
    std::vector<SyntaxError> errors;

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
    size_t pos_ = 0;
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

    // Manejo de errores.
    void report(const Token& token, const std::string& expected,
                const std::string& customMessage = "");
    void synchronize();
    void skipUnsupportedCommand(const std::string& name);

    // No terminales de la gramática.
    void parseListaFunciones();
    void parseDeclaracionFuncion();
    void parseTipoRetorno();
    void parseTipo();
    void parseTipoBase();
    void parseDimension();
    void parseParametros();
    void parseListaParametros();
    void parsePrincipal();
    void parseBloque();
    void parseListaVariables();
    void parseListaId();
    void parseComandos();
    void parseComando();
    void parseAsignacion();
    void parseComplemento();
    void parseFuncion();
    void parseArgumentos();

    // Expresiones, en orden de precedencia.
    void parseExpresion();
    void parseExpresionO();
    void parseExpresionY();
    void parseExpresionRelacional();
    void parseExpresionAditiva();
    void parseExpresionMultiplicativa();
    void parseFactor();
    void parseTermino();
    void parseDimension2();

    // Auxiliares de decisión.
    bool isTypeStart() const;
    bool isPrincipalStart() const;
    std::string tokenDescription(const Token& token) const;
};

#endif // PARSER_H
