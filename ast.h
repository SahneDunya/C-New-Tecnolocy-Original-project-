#ifndef CNT_COMPILER_AST_H
#define CNT_COMPILER_AST_H

#include "token.h" // Token konum bilgisi için

#include <vector>
#include <memory> // std::unique_ptr için
#include <string>

// Token'ın konum bilgisini temsil eden yapı (Token'dan veya ayrı bir yapıdan alabiliriz)
// Eğer Token yapınız zaten location içeriyorsa bunu kullanabilirsiniz,
// veya daha esnek bir yapı için ayrı bir struct tanımlayabilirsiniz:
struct TokenLocation {
    std::string filename;
    int line;
    int column;
    // Varsayılan kurucu
    TokenLocation() : filename(""), line(0), column(0) {}
    // Token'dan kurucu
    TokenLocation(const Token& t) : filename(t.filename), line(t.line), column(t.column) {}
    TokenLocation(std::string fn, int l, int c) : filename(std::move(fn)), line(l), column(c) {}
};


// AST düğümlerinin temel sınıfı
// Her AST düğümü, kaynak kodundaki konumunu tutabilir (hata raporlama için)
struct ASTNode {
    TokenLocation location; // Kaynak kodundaki başlangıç konumu

    // Sanal yıkıcı (virtual destructor) polymorphic delete için gereklidir
    virtual ~ASTNode() = default;

    // Hata ayıklama için düğüm türünü döndüren sanal metod
    virtual std::string getNodeType() const = 0; // Saf sanal fonksiyon

    // (İsteğe bağlı) Bir Visitor pattern uygulamayı düşünüyorsanız sanal accept metodu
     virtual void accept(ASTVisitor& visitor) = 0;
};

// Programın tamamını temsil eden kök düğüm
// declarations.h'tan DeclarationAST'i kullanır
struct DeclarationAST; // İleri bildirim

struct ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<DeclarationAST>> declarations; // Fonksiyonlar, structlar, enumlar vb.

    std::string getNodeType() const override { return "ProgramAST"; }

    // Çocuk düğümleri eklemek için yardımcı metod (isteğe bağlı)
    void addDeclaration(std::unique_ptr<DeclarationAST> decl); // Implementasyonu .cpp dosyasında
};

// Diğer temel AST düğüm kategorileri
// Bunlar sadece arayüzlerdir, somut sınıflar diğer başlıklarda olacak.
struct StatementAST : public ASTNode {
    virtual std::string getNodeType() const override { return "StatementAST"; }
};

struct ExpressionAST : public ASTNode {
    virtual std::string getNodeType() const override { return "ExpressionAST"; }
};

struct DeclarationAST : public ASTNode {
     virtual std::string getNodeType() const override { return "DeclarationAST"; }
};

struct TypeAST : public ASTNode {
     virtual std::string getNodeType() const override { return "TypeAST"; }
};


#endif // CNT_COMPILER_AST_H
