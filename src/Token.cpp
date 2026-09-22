#include "Token.h"
#include <vector>
#include <string>

struct SymbolEntry {
    std::string name;
    std::string type;       // Keyword, ID, Number, String, etc.
    int firstPosition;
    int occurrences;
};

struct LexerOutput {
    std::vector<Token> tokens;
    std::vector<LexerError> errors;
    std::vector<SymbolEntry> symbolTable;
};
