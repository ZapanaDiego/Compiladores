#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct SymbolEntry {
    std::string name;
    std::string type;        // "ID" por ahora (podrías ampliar a KEYWORD, etc.)
    std::size_t line;
    std::size_t col;
    std::size_t occurrences;
};

class SymbolTable {
public:
    void insert_or_update(const std::string& name, std::size_t line, std::size_t col);
    std::vector<SymbolEntry> to_vector() const;

private:
    std::unordered_map<std::string, SymbolEntry> symbols;
};