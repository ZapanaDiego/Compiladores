#ifndef AST_H
#define AST_H

#include "Token.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Tipos de nodo del árbol de sintaxis abstracta.
enum class ASTKind {
    Program,
    Function,
    Main,
    Param,
    Block,
    VarDecl,
    Declarator,
    Type,
    Dimension,
    Assign,
    Read,
    Write,
    If,
    While,
    Return,
    Call,
    Binary,
    Unary,
    Literal,
    Identifier,
    Index
};

struct ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

// Cada hijo lleva un rol para que el árbol sea autoexplicativo:
// "condition", "then", "else", "body", "argument", "left", etc.
struct ASTChild {
    std::string role;
    ASTNodePtr node;
};

struct ASTNode {
    ASTKind kind = ASTKind::Program;
    std::string value;
    std::string valueType;
    Token token;
    std::vector<ASTChild> children;

    std::string kindName() const;
    std::string to_json() const;
    std::string to_text(int indent = 0) const;
    std::size_t countNodes() const;
};

std::string astKindToString(ASTKind kind);

ASTNodePtr makeNode(ASTKind kind, const Token& token,
                    const std::string& value = "",
                    const std::string& valueType = "");

void addChild(const ASTNodePtr& parent, const std::string& role,
              const ASTNodePtr& child);

#endif // AST_H
