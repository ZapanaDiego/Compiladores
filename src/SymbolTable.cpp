#include "SymbolTable.h"

SymbolTable::SymbolTable() {
}

void SymbolTable::addSymbol(const std::string& name, const std::string& type) {
    table[name] = type;
}

bool SymbolTable::hasSymbol(const std::string& name) const {
    return table.find(name) != table.end();
}
