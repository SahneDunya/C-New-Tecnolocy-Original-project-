#ifndef CNT_COMPILER_EXPRESSIONS_H
#define CNT_COMPILER_EXPRESSIONS_H

#include "ast.h" // Temel AST sınıfları için (ExpressionAST artık resolvedSemanticType içeriyor)
#include "token.h" // Token::Type için
#include "symbol_table.h" // SymbolInfo için (ileri bildirim yukarıda yapıldı)
#include "type_system.h"  // Type* için (ileri bildirim yukarıda yapıldı)


#include <string>
#include <vector>
#include <memory> // std::unique_ptr için


// Literal (Değişmez) İfadeler
// resolvedSemanticType ExpressionAST'ten miras alınır.
struct IntLiteralAST : public ExpressionAST {
    int value;
    IntLiteralAST(int val, TokenLocation loc) : value(val) { location = loc; }
    std::string getNodeType() const override { return "IntLiteralAST"; }
};

struct FloatLiteralAST : public ExpressionAST {
    float value; // veya double
    FloatLiteralAST(float val, TokenLocation loc) : value(val) { location = loc; }
    std::string getNodeType() const override { return "FloatLiteralAST"; }
};

struct StringLiteralAST : public ExpressionAST {
    std::string value; // Tırnaklar hariç içerik (escape sequence'ler işlenmiş olmalı)
    StringLiteralAST(std::string val, TokenLocation loc) : value(std::move(val)) { location = loc; }
    std::string getNodeType() const override { return "StringLiteralAST"; }
};

struct CharLiteralAST : public ExpressionAST {
    char value; // Escape sequence işlenmiş tek karakter
    CharLiteralAST(char val, TokenLocation loc) : value(val) { location = loc; }
    std::string getNodeType() const override { return "CharLiteralAST"; }
};

struct BoolLiteralAST : public ExpressionAST {
    bool value;
    BoolLiteralAST(bool val, TokenLocation loc) : value(val) { location = loc; }
    std::string getNodeType() const override { return "BoolLiteralAST"; }
};

// Tanımlayıcı (Değişken, Fonksiyon Adı vb.)
struct IdentifierAST : public ExpressionAST {
    std::string name;

    // Semantic Analiz sonucu
    SymbolInfo* resolvedSymbol = nullptr; // SEMA tarafından çözülmüş sembol bilgisi
    // resolvedSemanticType zaten ExpressionAST'ten miras alınır ve sembolün tipini tutar.


    IdentifierAST(std::string n, TokenLocation loc) : name(std::move(n)) { location = loc; }
    std::string getNodeType() const override { return "IdentifierAST"; }
};


// İkili Operatör İfadeleri (a + b, x > y, p == q, etc.)
// resolvedSemanticType ExpressionAST'ten miras alınır.
struct BinaryOpAST : public ExpressionAST {
    Token::Type op; // Operatörün türü
    std::unique_ptr<ExpressionAST> left;
    std::unique_ptr<ExpressionAST> right;

    BinaryOpAST(Token::Type o, std::unique_ptr<ExpressionAST> l, std::unique_ptr<ExpressionAST> r, TokenLocation loc)
        : op(o), left(std::move(l)), right(std::move(r)) { location = loc; }

    std::string getNodeType() const override { return "BinaryOpAST"; }
};

// Tekli Operatör İfadeleri (!x, -y, &z, &mut w, *ptr)
// resolvedSemanticType ExpressionAST'ten miras alınır.
struct UnaryOpAST : public ExpressionAST {
    Token::Type op; // Operatörün türü
    std::unique_ptr<ExpressionAST> operand;

    UnaryOpAST(Token::Type o, std::unique_ptr<ExpressionAST> operand_node, TokenLocation loc)
        : op(o), operand(std::move(operand_node)) { location = loc; }

    std::string getNodeType() const override { return "UnaryOpAST"; }
};

// Atama İfadesi (sol_taraf = sag_taraf)
// resolvedSemanticType ExpressionAST'ten miras alınır (genellikle sağ tarafın tipi veya void).
struct AssignmentAST : public ExpressionAST {
     std::unique_ptr<ExpressionAST> left; // Sol taraf (atanabilir olmalı)
     std::unique_ptr<ExpressionAST> right; // Sağ taraf

     AssignmentAST(std::unique_ptr<ExpressionAST> l, std::unique_ptr<ExpressionAST> r, TokenLocation loc)
        : left(std::move(l)), right(std::move(r)) { location = loc; }

     std::string getNodeType() const override { return "AssignmentAST"; }
};

// Fonksiyon Çağrısı
struct CallExpressionAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> callee; // Çağrılan ifade (Fonksiyon adı veya method erişimi vb.)
    std::vector<std::unique_ptr<ExpressionAST>> arguments; // Argüman listesi

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedCalleeSymbol = nullptr; // Çözülmüş fonksiyon/metot sembolü
    // resolvedSemanticType ExpressionAST'ten miras alınır (fonksiyonun dönüş tipi).


    CallExpressionAST(std::unique_ptr<ExpressionAST> c, std::vector<std::unique_ptr<ExpressionAST>> args, TokenLocation loc)
        : callee(std::move(c)), arguments(std::move(args)) { location = loc; }

    std::string getNodeType() const override { return "CallExpressionAST"; }
};

// Üye Erişimi (obj.field, obj.method()) veya Yol Erişimi (Module::item, Enum::Variant)
struct MemberAccessAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> base; // Üyeye erişilen nesne veya modül ifadesi
    std::unique_ptr<IdentifierAST> member; // Üye adı (IdentifierAST)

    // Semantic Analiz sonuçları
    // Üyenin kendisine ait sembol bilgisi (StructField, EnumVariant, Function/Method)
    SymbolInfo* resolvedMemberSymbol = nullptr;
    // Üyenin tipi (resolvedSemanticType ExpressionAST'ten miras alınır)


    MemberAccessAST(std::unique_ptr<ExpressionAST> b, std::unique_ptr<IdentifierAST> m, TokenLocation loc)
        : base(std::move(b)), member(std::move(m)) { location = loc; }

    std::string getNodeType() const override { return "MemberAccessAST"; }
};

// Dizi/Index Erişimi (array[index])
struct IndexAccessAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> base; // Erişilen dizi veya koleksiyon ifadesi
    std::unique_ptr<ExpressionAST> index; // Index ifadesi

     // Semantic Analiz sonuçları
    /// resolvedSemanticType ExpressionAST'ten miras alınır (eleman tipi).


    IndexAccessAST(std::unique_ptr<ExpressionAST> b, std::unique_ptr<ExpressionAST> i, TokenLocation loc)
        : base(std::move(b)), index(std::move(i)) { location = loc; }

    std::string getNodeType() const override { return "IndexAccessAST"; }
};


// Match İfadesi (match expression { arm => result, ... })
// resolvedSemanticType ExpressionAST'ten miras alınır (tüm arm sonuçlarının ortak tipi).
struct MatchArmAST : public ASTNode {
    // Pattern kısmı daha sonra detaylandırılacak (literaller, değişken bağlama, enum varyantları vb.)
    // Şimdilik basit bir Placeholder/Expression/Literal varsayalım
    std::unique_ptr<ASTNode> pattern; // Eşleşme paterni AST (PatternAST'ten miras alanlar)
    std::unique_ptr<ExpressionAST> result; // Eşleşme durumunda çalışacak ifade

    // Semantic Analiz sonuçları
    // Pattern'ın beklediği tip SEMA tarafından belirlenmeli ve pattern düğümüne eklenmeli.
    // result->resolvedSemanticType tüm arm'lar arasında uyumlu olmalı.


    MatchArmAST(std::unique_ptr<ASTNode> p, std::unique_ptr<ExpressionAST> r, TokenLocation loc)
        : pattern(std::move(p)), result(std::move(r)) { location = loc; }

    std::string getNodeType() const override { return "MatchArmAST"; }
};

struct MatchExpressionAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> value; // Eşleşmenin yapılacağı ifade
    std::vector<std::unique_ptr<MatchArmAST>> arms; // Eşleşme kolları

    MatchExpressionAST(std::unique_ptr<ExpressionAST> val, std::vector<std::unique_ptr<MatchArmAST>> a, TokenLocation loc)
        : value(std::move(val)), arms(std::move(a)) { location = loc; }

    std::string getNodeType() const override { return "MatchExpressionAST"; }

    void addArm(std::unique_ptr<MatchArmAST> arm) {
        arms.push_back(std::move(arm));
    }
};


// Parantezli İfade - resolvedSemanticType iç ifadenin tipidir.
 struct ParenthesizedExpressionAST : public ExpressionAST { ... };


#endif // CNT_COMPILER_EXPRESSIONS_H
