#include "utils.h"
#include <sstream> // String stream için

// --- String Utilities ---

std::string cnt_compiler::trim(const std::string& str) {
    const auto ws = " \t\n\r\f\v";
    const auto first_char = str.find_first_not_of(ws);
    if (first_char == std::string::npos) {
        return ""; // Tamamı boşluk
    }
    const auto last_char = str.find_last_not_of(ws);
    return str.substr(first_char, (last_char - first_char + 1));
}

std::vector<std::string> cnt_compiler::splitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start)); // Son token'ı ekle
    return tokens;
}

std::string cnt_compiler::joinStrings(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < strings.size(); ++i) {
        ss << strings[i];
        if (i < strings.size() - 1) {
            ss << delimiter;
        }
    }
    return ss.str();
}

std::string cnt_compiler::toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_str;
}

std::string cnt_compiler::toUpper(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return upper_str;
}


// --- Location and Debug Utilities ---

std::string cnt_compiler::getLocationString(const TokenLocation& loc) {
    if (loc.line == -1) {
        return "(Unknown Location)"; // Geçersiz konum
    }
    return loc.filename + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
}

void cnt_compiler::printToken(std::ostream& os, const Token& token) {
    os << getLocationString(token.location)
       << " - Type: " << token.getTypeString()
       << ", Lexeme: '" << token.lexeme << "'"
       << std::endl;
}

// Bir AST düğümünü ve alt düğümlerini çıktı akışına yazdırır (hata ayıklama için)
void cnt_compiler::printASTNode(std::ostream& os, const ASTNode* node, int indent) {
    if (!node) {
        os << std::string(indent * 2, ' ') << "nullptr" << std::endl;
        return;
    }

    os << std::string(indent * 2, ' ')
       << node->getNodeType()
       << " at " << getLocationString(node->location)
       << std::endl;

    // Bu fonksiyonun her AST düğüm türü için çocuk düğümlerini nasıl gezmesi gerektiğini
    // belirlemek gerekir. Bu, dynamic_cast kullanarak veya bir Visitor pattern ile yapılabilir.
    // Örnek olarak, ProgramAST ve BlockStatementAST'in çocuklarını gezelim:

    if (const ProgramAST* program = dynamic_cast<const ProgramAST*>(node)) {
        for (const auto& decl_ptr : program->declarations) {
            printASTNode(os, decl_ptr.get(), indent + 1);
        }
    } else if (const BlockStatementAST* block = dynamic_cast<const BlockStatementAST*>(node)) {
        for (const auto& stmt_ptr : block->statements) {
            printASTNode(os, stmt_ptr.get(), indent + 1);
        }
    }
    // Diğer AST düğüm türleri için de benzer şekilde çocuklarını gezme kodunu ekleyin.
    // Örnek: BinaryOpAST -> left, right; FunctionDeclAST -> body, arguments vb.
     else if (const BinaryOpAST* binOp = dynamic_cast<const BinaryOpAST*>(node)) {
         os << std::string((indent + 1) * 2, ' ') << "Left:" << std::endl;
         printASTNode(os, binOp->left.get(), indent + 2);
         os << std::string((indent + 1) * 2, ' ') << "Right:" << std::endl;
         printASTNode(os, binOp->right.get(), indent + 2);
     }
    // ...
}


// --- File System Utilities ---

std::string cnt_compiler::getFileExtension(const std::filesystem::path& path) {
    return path.extension().string();
}

std::string cnt_compiler::getFileNameWithoutExtension(const std::filesystem::path& path) {
    return path.stem().string();
}

bool cnt_compiler::fileExists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

std::filesystem::path cnt_compiler::normalizePath(const std::filesystem::path& path) {
     return std::filesystem::canonical(path);
}

// --- Other General Utilities ---
// Buraya diğer implementasyonlar eklenecek.
