#ifndef CNT_COMPILER_AST_H
#define CNT_COMPILER_AST_H

#include "token.h" // Token konum bilgisi için

#include <vector>
#include <memory> // std::unique_ptr için
#include <string>

// AST düğümlerinin temel sınıfı
// Her AST düğümü, kaynak kodundaki konumunu tutabilir (hata raporlama için)
struct ASTNode {
    TokenLocation location; // Token veya ilk token'ın konumu

    // Sanal yıkıcı (virtual destructor) polymorphic delete için gereklidir
    virtual ~ASTNode() = default;

    // Hata ayıklama için düğüm türünü döndüren sanal metod
    virtual std::string getNodeType() const = 0; // Saf sanal fonksiyon
};

// Programın tamamını temsil eden kök düğüm
struct ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> declarations; // Fonksiyonlar, structlar, enumlar vb.

    std::string getNodeType() const override { return "ProgramAST"; }

    // Çocuk düğümleri eklemek için yardımcı metod (isteğe bağlı)
    void addDeclaration(std::unique_ptr<ASTNode> decl) {
        declarations.push_back(std::move(decl));
    }
};

// Diğer temel AST düğüm kategorileri (Bunlar daha sonra ayrı dosyalara ayrılabilir)
// Bildirimler (Declarations): Fonksiyon, Struct, Enum, Global Değişken
struct DeclarationAST : public ASTNode {
     virtual std::string getNodeType() const override { return "DeclarationAST"; }
};

// Deyimler (Statements): Atama, Kontrol Yapıları (if, match), Döngüler, Return, Break, Continue, Import
struct StatementAST : public ASTNode {
    virtual std::string getNodeType() const override { return "StatementAST"; }
};

// İfadeler (Expressions): Aritmetik, Mantıksal, Karşılaştırma, Fonksiyon Çağrısı, Değişmezler
struct ExpressionAST : public ASTNode {
    virtual std::string getNodeType() const override { return "ExpressionAST"; }
};

// Tip İfadeleri (Types): int, float, MyStruct, &T, &mut T
struct TypeAST : public ASTNode {
     virtual std::string getNodeType() const override { return "TypeAST"; }
};


// Örnek: Basit bir Integer Literal AST düğümü
struct IntLiteralAST : public ExpressionAST {
    int value; // Değişmezin değeri

    IntLiteralAST(int val, TokenLocation loc) : value(val) { location = loc; }

     std::string getNodeType() const override { return "IntLiteralAST"; }
};

// Örnek: Basit bir Binary Operatör AST düğümü (a + b gibi)
struct BinaryOpAST : public ExpressionAST {
    Token::Type op; // Operatörün token türü (+, -, * vb.)
    std::unique_ptr<ExpressionAST> left; // Sol operand
    std::unique_ptr<ExpressionAST> right; // Sağ operand

    BinaryOpAST(Token::Type o, std::unique_ptr<ExpressionAST> l, std::unique_ptr<ExpressionAST> r, TokenLocation loc)
        : op(o), left(std::move(l)), right(std::move(r)) { location = loc; }

     std::string getNodeType() const override { return "BinaryOpAST"; }
};

// Not: Dilinizin tüm sözdizimsel yapıları için (fonksiyonlar, structlar, match ifadeleri vb.)
// ilgili AST düğümlerini burada veya ayrı başlıklarda tanımlamanız gerekecektir.
// Örn: FunctionDeclAST, MatchExpressionAST, IfStatementAST, ImportStatementAST vb.

#endif // CNT_COMPILER_AST_H
