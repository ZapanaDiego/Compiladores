#pragma once
#include <string>

struct Token {
    std::string type;
    std::string lexeme;
    int line;
    int column;
};
