#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct SymbolEntry {
    std::string name;
    std::string token_type;
    std::size_t first_line;
    std::size_t first_col;
    std::size_t occurrences;
};

struct SymbolTable {
    std::unordered_map<std::string, SymbolEntry> symbols;
    
    void insert_or_update(const std::string& name, std::size_t line, std::size_t col);
    std::vector<SymbolEntry> to_vector() const;
};
