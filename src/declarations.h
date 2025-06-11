#ifndef CNT_COMPILER_DECLARATIONS_H
#define CNT_COMPILER_DECLARATIONS_H

#include "ast.h" // Temel AST sınıfları için
#include "expressions.h" // Bildirimler ifade içerebilir (değişken başlangıç değeri, fonksiyon gövdesi vb.)
#include "statements.h"  // Fonksiyon gövdesi blok deyimidir
#include "types.h"       // Tür bilgileri için (parametre türleri, dönüş türleri, alan türleri)
#include "symbol_table.h" // SymbolInfo için (ileri bildirim yukarıda yapıldı)
#include "type_system.h" // Type* için (ileri bildirim yukarıda yapıldı)


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
    bool isPublic = false; // <-- isPublic bayrağı

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedSymbol = nullptr; // SEMA tarafından çözülmüş sembol bilgisi
    // Type* resolvedSemanticType zaten ExpressionAST'ten miras alındığı için burada da geçerli (ama varDecl bir ifade değil, bildirim, dikkat!)
    // VarDecl'in kendisinin değil, DEĞERİNE ait tipi vardır.
     std::unique_ptr<ExpressionAST> initializer -> initializer->resolvedSemanticType'ı kullanın.
    // veya sembolün tipini SymbolInfo*->type'tan alın.


    VarDeclAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, std::unique_ptr<ExpressionAST> init, bool mut, bool pub, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)), initializer(std::move(init)), isMutable(mut), isPublic(pub) { location = loc; }
        // Parser'dan gelen isPublic ve isGlobal bayrakları burada Constructor'a eklenmeli.
        // Şu anki parser isGlobal'i ayrı alıyor, AST'de VarDecl'in kendisinde isGlobal tutulabilir.
         bool isGlobal = false; // <-- isGlobal bayrağı da eklenebilir VarDeclAST'e.


    std::string getNodeType() const override { return "VarDeclAST"; }
};


// Fonksiyon Bildirimi (fn name(args) -> return_type { body })
struct FunctionArgAST : public ASTNode { // Fonksiyon argümanı, DeclarationAST'ten miras almayabilir
    std::unique_ptr<IdentifierAST> name; // Argüman ismi
    std::unique_ptr<TypeAST> type;       // Argüman türü AST düğümü
    bool isMutable; // Argüman 'mut' olabilir mi? (Rust'taki gibi)

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedSymbol = nullptr; // Argüman için SEMA tarafından çözülmüş sembol bilgisi
     Type* resolvedSemanticType; // resolvedSemanticType aslında type->resolvedSemanticType'tan alınır.


    FunctionArgAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, bool mut, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)), isMutable(mut) { location = loc; }

    std::string getNodeType() const override { return "FunctionArgAST"; }
};


struct FunctionDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Fonksiyon ismi
    std::vector<std::unique_ptr<FunctionArgAST>> arguments; // Argüman listesi AST düğümleri
    std::unique_ptr<TypeAST> returnType; // Dönüş türü AST düğümü (eğer belirtilmişse)
    std::unique_ptr<BlockStatementAST> body; // Fonksiyon gövdesi
    bool isPublic = false; // <-- isPublic bayrağı

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedSymbol = nullptr; // Fonksiyon için SEMA tarafından çözülmüş sembol bilgisi
     Type* resolvedSemanticType; // FunctionType* olarak, argüman ve dönüş tipleri bu Type objesinde tutulur.
                                // Veya FunctionType bilgisi SymbolInfo*->type'ta tutulur.


    FunctionDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<FunctionArgAST>> args, std::unique_ptr<TypeAST> ret_t, std::unique_ptr<BlockStatementAST> b, bool pub, TokenLocation loc)
        : name(std::move(n)), arguments(std::move(args)), returnType(std::move(ret_t)), body(std::move(b)), isPublic(pub) { location = loc; }

    std::string getNodeType() const override { return "FunctionDeclAST"; }
};


// Struct Bildirimi (struct Name { fields })
struct StructFieldAST : public ASTNode { // Struct alanı, DeclarationAST'ten miras almayabilir
    std::unique_ptr<IdentifierAST> name; // Alan ismi
    std::unique_ptr<TypeAST> type;       // Alan türü AST düğümü
     bool isPublic; // Erişim belirleyici (varsa, alanlar da public/private olabilir)

    // Semantic Analiz sonuçları
     Type* resolvedSemanticType; // type->resolvedSemanticType'tan alınır.
     SymbolInfo* resolvedSymbol; // Alan için SymbolInfo (eğer sembol tablosunda alanlar da tutuluyorsa)

    StructFieldAST(std::unique_ptr<IdentifierAST> n, std::unique_ptr<TypeAST> t, TokenLocation loc)
        : name(std::move(n)), type(std::move(t)) { location = loc; }

     std::string getNodeType() const override { return "StructFieldAST"; }
};

struct StructDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Struct ismi
    std::vector<std::unique_ptr<StructFieldAST>> fields; // Alan listesi
    bool isPublic = false; // <-- isPublic bayrağı

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedSymbol = nullptr; // Struct için SEMA tarafından çözülmüş sembol bilgisi
    Type* resolvedSemanticType = nullptr; // StructType* olarak, alan bilgisi bu Type objesinde tutulur.


    StructDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<StructFieldAST>> f, bool pub, TokenLocation loc)
        : name(std::move(n)), fields(std::move(f)), isPublic(pub) { location = loc; }

    std::string getNodeType() const override { return "StructDeclAST"; }
};

// Enum Bildirimi (enum Name { Variants })
// Basit enum'lar için
struct EnumVariantAST : public ASTNode { // Enum varyantı, DeclarationAST'ten miras almayabilir
     std::unique_ptr<IdentifierAST> name; // Varyant ismi
     // İsteğe bağlı olarak ilişkilendirilmiş değerler/tipler (enum Option<T> { Some(T), None })
      std::vector<std::unique_ptr<TypeAST>> associatedTypes; // Eğer tipleri varsa

    // Semantic Analiz sonuçları
     SymbolInfo* resolvedSymbol; // Varyant için SymbolInfo
     Type* resolvedSemanticType; // Eğer varyantın ilişkili tipleri varsa, TupleType* gibi

     EnumVariantAST(std::unique_ptr<IdentifierAST> n, TokenLocation loc) : name(std::move(n)) { location = loc; }
     std::string getNodeType() const override { return "EnumVariantAST"; }
};

struct EnumDeclAST : public DeclarationAST {
    std::unique_ptr<IdentifierAST> name; // Enum ismi
    std::vector<std::unique_ptr<EnumVariantAST>> variants; // Varyant listesi
    bool isPublic = false; // <-- isPublic bayrağı

    // Semantic Analiz sonuçları
    SymbolInfo* resolvedSymbol = nullptr; // Enum için SEMA tarafından çözülmüş sembol bilgisi
    Type* resolvedSemanticType = nullptr; // EnumType* olarak, varyant bilgisi bu Type objesinde tutulur.


    EnumDeclAST(std::unique_ptr<IdentifierAST> n, std::vector<std::unique_ptr<EnumVariantAST>> v, bool pub, TokenLocation loc)
        : name(std::move(n)), variants(std::move(v)), isPublic(pub) { location = loc; }

    std::string getNodeType() const override { return "EnumDeclAST"; }
};


// Trait (Arayüz) Bildirimi - Otomatik Arayüz Çıkarımı ile ilişkili olabilir
 struct TraitDeclAST : public DeclarationAST {
      std::unique_ptr<IdentifierAST> name;
//      // Fonksiyon imzaları, associated types vb.
      bool isPublic = false; // <-- isPublic bayrağı
      SymbolInfo* resolvedSymbol = nullptr;
      Type* resolvedSemanticType = nullptr; // TraitType*
 };

// Implementation Bloğu (Impl)
 struct ImplBlockAST : public DeclarationAST {
      std::unique_ptr<TypeAST> targetType; // Implementasyonun yapıldığı tür AST
       std::unique_ptr<TypeAST> implementedTrait; // İmplemente edilen trait AST (varsa)
      std::vector<std::unique_ptr<FunctionDeclAST>> methodsAndAssociatedFunctions; // Metotlar/Fonksiyonlar
//      // Impl bloklarının kendisi genellikle public olmaz, içindeki üyeler public olur.
       bool isPublic = false; // <-- isPublic bayrağı Impl bloğunun kendisine değil, içindeki methodlara gelir.

      Type* resolvedTargetSemanticType = nullptr; // targetType->resolvedSemanticType'tan alınır
      Type* resolvedImplementedTraitType = nullptr; // implementedTrait->resolvedSemanticType'tan alınır
 };


#endif // CNT_COMPILER_DECLARATIONS_H
