#ifndef CNT_COMPILER_EXPRESSIONS_H
#define CNT_COMPILER_EXPRESSIONS_H

#include "ast.h" // Temel AST sınıfları için
#include "token.h" // Token::Type için

#include <string>
#include <vector>
#include <memory> // std::unique_ptr için

// Literal (Değişmez) İfadeler
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
    IdentifierAST(std::string n, TokenLocation loc) : name(std::move(n)) { location = loc; }
    std::string getNodeType() const override { return "IdentifierAST"; }
};


// İkili Operatör İfadeleri (a + b, x > y, p == q, etc.)
struct BinaryOpAST : public ExpressionAST {
    Token::Type op; // Operatörün türü
    std::unique_ptr<ExpressionAST> left;
    std::unique_ptr<ExpressionAST> right;

    BinaryOpAST(Token::Type o, std::unique_ptr<ExpressionAST> l, std::unique_ptr<ExpressionAST> r, TokenLocation loc)
        : op(o), left(std::move(l)), right(std::move(r)) { location = loc; }

    std::string getNodeType() const override { return "BinaryOpAST"; }
};

// Tekli Operatör İfadeleri (!x, -y, &z, &mut w, *ptr)
struct UnaryOpAST : public ExpressionAST {
    Token::Type op; // Operatörün türü
    std::unique_ptr<ExpressionAST> operand;

    UnaryOpAST(Token::Type o, std::unique_ptr<ExpressionAST> operand_node, TokenLocation loc)
        : op(o), operand(std::move(operand_node)) { location = loc; }

    std::string getNodeType() const override { return "UnaryOpAST"; }
};

// Atama İfadesi (sol_taraf = sag_taraf) - Aslında bir BinaryOp gibi düşünülebilir ama özel ele almak faydalı olabilir.
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

    CallExpressionAST(std::unique_ptr<ExpressionAST> c, std::vector<std::unique_ptr<ExpressionAST>> args, TokenLocation loc)
        : callee(std::move(c)), arguments(std::move(args)) { location = loc; }

    std::string getNodeType() const override { return "CallExpressionAST"; }
};

// Üye Erişimi (obj.field, obj.method()) veya Yol Erişimi (Module::item, Enum::Variant)
struct MemberAccessAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> base; // Üyeye erişilen nesne veya modül
    std::unique_ptr<IdentifierAST> member; // Üye adı

    // Eğer metot çağrısı ise:
     bool isMethodCall = false;
     std::vector<std::unique_ptr<ExpressionAST>> methodArguments;

    MemberAccessAST(std::unique_ptr<ExpressionAST> b, std::unique_ptr<IdentifierAST> m, TokenLocation loc)
        : base(std::move(b)), member(std::move(m)) { location = loc; }

    std::string getNodeType() const override { return "MemberAccessAST"; }
};

// Dizi/Index Erişimi (array[index])
struct IndexAccessAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> base; // Erişilen dizi veya koleksiyon
    std::unique_ptr<ExpressionAST> index; // Index ifadesi

    IndexAccessAST(std::unique_ptr<ExpressionAST> b, std::unique_ptr<ExpressionAST> i, TokenLocation loc)
        : base(std::move(b)), index(std::move(i)) { location = loc; }

    std::string getNodeType() const override { return "IndexAccessAST"; }
};


// Match İfadesi (match expression { arm => result, ... })
// match ifadesinin kendisi bir değer döndürdüğü için ExpressionAST'tir.
struct MatchArmAST : public ASTNode { // Match Arm, ExpressionAST'ten miras almayabilir, match ifadesinin iç yapı taşıdır
    // Pattern kısmı daha sonra detaylandırılacak (literaller, değişken bağlama, enum varyantları vb.)
    // Şimdilik basit bir IdentifierPattern varsayalım veya doğrudan Expression ile eşleştiğini düşünelim
    std::unique_ptr<ASTNode> pattern; // Eşleşme paterni (Örn: LiteralAST, IdentifierAST, EnumVariantPatternAST)
    std::unique_ptr<ExpressionAST> result; // Eşleşme durumunda çalışacak ifade

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


// Parantezli İfade (Aslında sadece bir gruplandırmadır, AST'de ayrı düğüm gerekmeyebilir)
// Parser, parantez içindeki ifadeyi ayrıştırıp doğrudan o ifadenin AST düğümünü döndürebilir.
// Eğer parantezlerin kendisini AST'de tutmak isterseniz:
 struct ParenthesizedExpressionAST : public ExpressionAST {
    std::unique_ptr<ExpressionAST> inner;
    ParenthesizedExpressionAST(std::unique_ptr<ExpressionAST> i, TokenLocation loc) : inner(std::move(i)) { location = loc; }
    std::string getNodeType() const override { return "ParenthesizedExpressionAST"; }
 };


#endif // CNT_COMPILER_EXPRESSIONS_H
