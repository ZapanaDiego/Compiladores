#include "SymbolTable.h"
#include <fstream>

int SymbolTable::insertOrGet(const std::string& lexeme) {
    auto it = symbols_map.find(lexeme);
    if (it != symbols_map.end()) {
        return it->second;
    }
    int index = symbols_list.size();
    symbols_list.push_back(lexeme);
    symbols_map[lexeme] = index;
    return index;
}

void SymbolTable::exportToFile(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (out.is_open()) {
        for (size_t i = 0; i < symbols_list.size(); ++i) {
            out << i << "\t" << symbols_list[i] << "\n";
        }
    }
}

std::string SymbolTable::to_json() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < symbols_list.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"pos\":" << i << ",\"id\":\"" << symbols_list[i] << "\"}";
    }
    oss << "]";
    return oss.str();
}
