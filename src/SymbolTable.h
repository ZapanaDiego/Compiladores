#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

class SymbolTable {
private:
    std::vector<std::string> symbols_list;
    std::unordered_map<std::string, int> symbols_map;

public:
    int insertOrGet(const std::string& lexeme);
    void exportToFile(const std::string& filepath) const;
    std::string to_json() const;
};

#endif
