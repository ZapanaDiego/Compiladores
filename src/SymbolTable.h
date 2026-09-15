#pragma once
#include <string>
#include <unordered_map>
#include "Token.h"

class SymbolTable {
public:
    SymbolTable();
    void addSymbol(const std::string& name, const std::string& type);
    bool hasSymbol(const std::string& name) const;

private:
    std::unordered_map<std::string, std::string> table;
};
