#ifndef CNT_COMPILER_DECLARATIONS_H
#define CNT_COMPILER_DECLARATIONS_H

#include "ast.h" // Temel AST sınıfları için
#include "expressions.h" // Bildirimler ifade içerebilir (değişken başlangıç değeri, fonksiyon gövdesi vb.)
#include "statements.h"  // Fonksiyon gövdesi blok deyimidir
#include "types.h"       // Tür bilgileri için (parametre türleri, dönüş türleri, alan türleri)

#include <vector>
#include <memory> // std::unique_ptr için
#include <string>

// Değişken Bildirimi (let name: Type = value;)
// Hem global hem de yerel değişkenler için kullanılabilir
struct VarDeclAST : public DeclarationAST, public StatementAST { // Hem Declaration hem Statement olabilir
    std::unique_ptr<IdentifierAST> name; // Değişken ismi
    std::unique_ptr<TypeAST> type;       // Değişkenin türü (isteğe bağlı, tür çıkarımı varsa)
    std::unique_ptr<ExpressionAST> initializer; // Başlangıç değeri (isteğe bağlı)
    bool isMutable; // 'mut' anahtar kelimesi kullanıldı mı?

    VarDeclAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, std::unique_ptr<ExpressionAST> init, bool mut, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)), initializer(std::move(init)), isMutable(mut) { location = loc; }

    std::string getNodeType() const override { return "VarDeclAST"; }
};


// Fonksiyon Bildirimi (fn name(args) -> return_type { body })
struct FunctionArgAST : public ASTNode { // Fonksiyon argümanı, DeclarationAST'ten miras almayabilir
    std::unique_ptr<IdentifierAST> name; // Argüman ismi
    std::unique_ptr<TypeAST> type;       // Argüman türü
    bool isMutable; // Argüman 'mut' olabilir mi? (Rust'taki gibi)

    FunctionArgAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, bool mut, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)), isMutable(mut) { location = loc; }

    std::string getNodeType() const override { return "FunctionArgAST"; }
};


struct FunctionDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Fonksiyon ismi
    std::vector<std::unique_ptr<FunctionArgAST>> arguments; // Argüman listesi
    std::unique_ptr<TypeAST> returnType; // Dönüş türü (eğer belirtilmişse)
    std::unique_ptr<BlockStatementAST> body; // Fonksiyon gövdesi

    FunctionDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<FunctionArgAST>> args, std::unique_ptr<TypeAST> ret_t, std::unique_ptr<BlockStatementAST> b, TokenLocation loc)
        : name(std::move(n)), arguments(std::move(args)), returnType(std::move(ret_t)), body(std::move(b)) { location = loc; }

    std::string getNodeType() const override { return "FunctionDeclAST"; }
};


// Struct Bildirimi (struct Name { fields })
struct StructFieldAST : public ASTNode { // Struct alanı, DeclarationAST'ten miras almayabilir
    std::unique_ptr<IdentifierAST> name; // Alan ismi
    std::unique_ptr<TypeAST> type;       // Alan türü
     bool isPublic; // Erişim belirleyici (varsa)

    StructFieldAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)) { location = loc; }

     std::string getNodeType() const override { return "StructFieldAST"; }
};

struct StructDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Struct ismi
    std::vector<std::unique_ptr<StructFieldAST>> fields; // Alan listesi
    // Struct'a ait ilişkilendirilmiş fonksiyonlar (associated functions) veya methodlar
    // Bunlar ayrı Impl (Implementation) bloklarında tanımlanabilir.

    StructDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<StructFieldAST>> f, TokenLocation loc)
        : name(std::move(n)), fields(std::move(f)) { location = loc; }

    std::string getNodeType() const override { return "StructDeclAST"; }
};

// Enum Bildirimi (enum Name { Variants })
// Basit enum'lar için
struct EnumVariantAST : public ASTNode { // Enum varyantı, DeclarationAST'ten miras almayabilir
     std::unique_ptr<IdentifierAST> name; // Varyant ismi
     // İsteğe bağlı olarak ilişkilendirilmiş değerler/tipler (enum Option<T> { Some(T), None })
      std::vector<std::unique_ptr<TypeAST>> associatedTypes;

     EnumVariantAST(std::unique_ptr<IdentifierAST> n, TokenLocation loc) : name(std::move(n)) { location = loc; }
     std::string getNodeType() const override { return "EnumVariantAST"; }
};

struct EnumDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Enum ismi
    std::vector<std::unique_ptr<EnumVariantAST>> variants; // Varyant listesi

    EnumDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<EnumVariantAST>> v, TokenLocation loc)
        : name(std::move(n)), variants(std::move(v)) { location = loc; }

    std::string getNodeType() const override { return "EnumDeclAST"; }
};


// Trait (Arayüz) Bildirimi - Otomatik Arayüz Çıkarımı ile ilişkili olabilir
// Kullanıcının traitleri açıkça tanımlamasına izin verilebilir veya derleyici belirli desenlerden traitleri çıkarabilir.
 struct TraitDeclAST : public DeclarationAST {
      std::unique_ptr<IdentifierAST> name;
      std::vector<std::unique_ptr<FunctionSignatureAST>> requiredFunctions; // Trait'i implement edenlerin sağlaması gereken fonksiyon imzaları
//      // ... diğer gereksinimler (associated types, constants)
 };

// Implementation Bloğu (Impl) - Struct veya Trait için metot/ilişkilendirilmiş fonksiyon implementasyonları
 struct ImplBlockAST : public DeclarationAST {
      std::unique_ptr<TypeAST> targetType; // Implementasyonun yapıldığı tür (Struct, Enum vb.)
//      // İsteğe bağlı olarak implemente edilen trait (impl Trait for Type)
       std::unique_ptr<TypeAST> implementedTrait;
      std::vector<std::unique_ptr<FunctionDeclAST>> methodsAndAssociatedFunctions;
 };


#endif // CNT_COMPILER_DECLARATIONS_H
