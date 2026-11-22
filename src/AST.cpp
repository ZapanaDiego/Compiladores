#include "AST.h"

#include <sstream>

namespace {

std::string escape_json(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

void append_text(const ASTNode& node, std::ostringstream& oss, int indent) {
    oss << std::string(static_cast<std::size_t>(indent), ' ')
        << node.kindName();
    if (!node.value.empty()) {
        oss << " " << node.value;
    }
    if (!node.valueType.empty()) {
        oss << " (" << node.valueType << ")";
    }
    oss << "\n";

    for (const auto& child : node.children) {
        oss << std::string(static_cast<std::size_t>(indent + 2), ' ')
            << "[" << child.role << "]\n";
        if (child.node) {
            append_text(*child.node, oss, indent + 4);
        } else {
            oss << std::string(static_cast<std::size_t>(indent + 4), ' ')
                << "(null)\n";
        }
    }
}

} // namespace

std::string astKindToString(ASTKind kind) {
    switch (kind) {
        case ASTKind::Program:    return "Program";
        case ASTKind::Function:   return "Function";
        case ASTKind::Main:       return "Main";
        case ASTKind::Param:      return "Param";
        case ASTKind::Block:      return "Block";
        case ASTKind::VarDecl:    return "VarDecl";
        case ASTKind::Declarator: return "Declarator";
        case ASTKind::Type:       return "Type";
        case ASTKind::Dimension:  return "Dimension";
        case ASTKind::Assign:     return "Assign";
        case ASTKind::Read:       return "Read";
        case ASTKind::Write:      return "Write";
        case ASTKind::If:         return "If";
        case ASTKind::While:      return "While";
        case ASTKind::Return:     return "Return";
        case ASTKind::Call:       return "Call";
        case ASTKind::Binary:     return "Binary";
        case ASTKind::Unary:      return "Unary";
        case ASTKind::Literal:    return "Literal";
        case ASTKind::Identifier: return "Identifier";
        case ASTKind::Index:      return "Index";
    }
    return "Unknown";
}

ASTNodePtr makeNode(ASTKind kind, const Token& token,
                    const std::string& value,
                    const std::string& valueType) {
    auto node = std::make_shared<ASTNode>();
    node->kind = kind;
    node->token = token;
    node->value = value;
    node->valueType = valueType;
    return node;
}

void addChild(const ASTNodePtr& parent, const std::string& role,
              const ASTNodePtr& child) {
    if (parent && child) {
        parent->children.push_back(ASTChild{role, child});
    }
}

std::string ASTNode::kindName() const {
    return astKindToString(kind);
}

std::string ASTNode::to_json() const {
    std::ostringstream oss;
    oss << "{\"type\":\"" << escape_json(kindName()) << "\"";
    if (!value.empty()) {
        oss << ",\"value\":\"" << escape_json(value) << "\"";
    }
    if (!valueType.empty()) {
        oss << ",\"valueType\":\"" << escape_json(valueType) << "\"";
    }
    oss << ",\"line\":" << token.line
        << ",\"col\":" << token.col
        << ",\"children\":[";
    for (std::size_t i = 0; i < children.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"role\":\"" << escape_json(children[i].role)
            << "\",\"node\":";
        if (children[i].node) {
            oss << children[i].node->to_json();
        } else {
            oss << "null";
        }
        oss << "}";
    }
    oss << "]}";
    return oss.str();
}

std::string ASTNode::to_text(int indent) const {
    std::ostringstream oss;
    append_text(*this, oss, indent);
    return oss.str();
}

std::size_t ASTNode::countNodes() const {
    std::size_t total = 1;
    for (const auto& child : children) {
        if (child.node) {
            total += child.node->countNodes();
        }
    }
    return total;
}
