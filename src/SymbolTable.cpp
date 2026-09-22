#include "SymbolTable.h"

void SymbolTable::insert_or_update(const std::string& name, std::size_t line, std::size_t col) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        it->second.occurrences++;
    } else {
        symbols[name] = {name, "ID", line, col, 1};
    }
}

std::vector<SymbolEntry> SymbolTable::to_vector() const {
    std::vector<SymbolEntry> vec;
    vec.reserve(symbols.size());
    for (const auto& pair : symbols) {
        vec.push_back(pair.second);
    }
    return vec;
}
