#ifndef CNT_COMPILER_AST_H
#define CNT_COMPILER_AST_H

#include "token.h" // Token konum bilgisi (TokenLocation) için

#include <vector>
#include <memory> // std::unique_ptr için
#include <string>

// Semantic sistemden ileri bildirimler
struct Type; // semantic Type*
struct SymbolInfo; // semantic SymbolInfo*

// Token'ın konum bilgisini temsil eden yapı (Token'dan kopyalanır)
// Eğer Token yapınız zaten location içeriyorsa bunu kullanabilirsiniz.
// TokenLocation yapısı daha önceki adımda ast.h'a eklendi.
 struct TokenLocation { ... };

// AST düğümlerinin temel sınıfı
struct ASTNode {
    TokenLocation location; // Kaynak kodundaki başlangıç konumu

    // (Opsiyonel ama yaygın) İfade ve Tip düğümlerine semantik tip bilgisini eklemek
    // SEMA bu alanı doldurur.
    // Type* resolvedSemanticType = nullptr; // Semantic analiz sonrası çözülmüş tip bilgisi

    // Sanal yıkıcı (polymorphic delete için)
    virtual ~ASTNode() = default;

    // Hata ayıklama için düğüm türünü döndüren sanal metod
    virtual std::string getNodeType() const = 0;
};

// Programın tamamını temsil eden kök düğüm
// declarations.h'tan DeclarationAST'i kullanır
struct DeclarationAST; // İleri bildirim

struct ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<DeclarationAST>> declarations; // Fonksiyonlar, structlar, enumlar vb.

    std::string getNodeType() const override { return "ProgramAST"; }

    // Çocuk düğümleri eklemek için yardımcı metod
    void addDeclaration(std::unique_ptr<DeclarationAST> decl); // Implementasyonu .cpp dosyasında veya inline
};

// Diğer temel AST düğüm kategorileri
struct StatementAST : public ASTNode {
    virtual std::string getNodeType() const override { return "StatementAST"; }
};

struct ExpressionAST : public ASTNode {
    // ExpressionAST'ten miras alan tüm düğümler için çözülmüş semantik tipi tutar
    Type* resolvedSemanticType = nullptr; // Semantic analiz sonrası çözülmüş tip bilgisi

    virtual std::string getNodeType() const override { return "ExpressionAST"; }
};

struct DeclarationAST : public ASTNode {
    // isPublic bayrağı alt sınıflara eklenecek
     virtual std::string getNodeType() const override { return "DeclarationAST"; }
};

struct TypeAST : public ASTNode {
    // TypeAST'ten miras alan tüm düğümler için çözülmüş semantik tipi tutar
    // Parser sadece sözdizimsel tip ağacını kurar, SEMA bunu çözümlenmiş Type* objesine bağlar.
    Type* resolvedSemanticType = nullptr; // Semantic analiz sonrası çözülmüş semantik tip objesi

    virtual std::string getNodeType() const override { return "TypeAST"; }
};


#endif // CNT_COMPILER_AST_H
