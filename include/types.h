#ifndef CNT_COMPILER_TYPES_H
#define CNT_COMPILER_TYPES_H

#include "ast.h" // Temel AST sınıfları için (TypeAST artık resolvedSemanticType içeriyor)
#include "expressions.h" // Tip isimleri IdentifierAST olabilir (IdentifierAST resolvedSymbol içeriyor)


#include <vector>
#include <memory> // std::unique_ptr için
#include <string>


// Temel Tipler (int, float, bool, string veya kullanıcı tanımlı Struct/Enum adları)
// resolvedSemanticType TypeAST'ten miras alınır (IntType*, StructType* vb.).
struct BaseTypeAST : public TypeAST {
    std::string name; // "int", "float", "bool", "string", veya Struct/Enum adı lexeme'i
    // Eğer isim IdentifierAST olarak ayrıştırılıyorsa:
     std::unique_ptr<IdentifierAST> name_id; // IdentifierAST resolvedSymbol içerir.

    BaseTypeAST(std::string n, TokenLocation loc) : name(std::move(n)) { location = loc; }
     BaseTypeAST(std::unique_ptr<IdentifierAST> id, TokenLocation loc) : name_id(std::move(id)) { location = loc; }


    std::string getNodeType() const override { return "BaseTypeAST"; }
};

// Referans Tipleri (&T, &mut T)
// resolvedSemanticType TypeAST'ten miras alınır (ReferenceType*).
struct ReferenceTypeAST : public TypeAST {
    std::unique_ptr<TypeAST> referencedType; // Referansın gösterdiği tip AST düğümü
    bool isMutable; // &mut mu, & mi?

    // Semantic Analiz sonucu
    // referencedType->resolvedSemanticType'ı kullanın.

    ReferenceTypeAST(std::unique_ptr<TypeAST> referenced_t, bool mut, TokenLocation loc)
        : referencedType(std::move(referenced_t)), isMutable(mut) { location = loc; }

    std::string getNodeType() const override { return "ReferenceTypeAST"; }
};

// Dizi Tipleri ([T] veya [T; size])
// resolvedSemanticType TypeAST'ten miras alınır (ArrayType*).
struct ArrayTypeAST : public TypeAST {
    std::unique_ptr<TypeAST> elementType; // Dizi elemanlarının tipi AST düğümü
    // İsteğe bağlı: Sabit boyutlu diziler için boyut ifadesi AST
     std::unique_ptr<ExpressionAST> size; // Size ExpressionAST'i resolvedSemanticType (int) içerir.

    // Semantic Analiz sonucu
    // elementType->resolvedSemanticType'ı kullanın.
    // size->resolvedSemanticType int olmalı.

    ArrayTypeAST(std::unique_ptr<TypeAST> element_t, TokenLocation loc)
        : elementType(std::move(element_t)) { location = loc; }
     ArrayTypeAST(std::unique_ptr<TypeAST> element_t, std::unique_ptr<ExpressionAST> s, TokenLocation loc)
          elementType(std::move(element_t)), size(std::move(s)) { location = loc; }


    std::string getNodeType() const override { return "ArrayTypeAST"; }
};

// Pointer Tipleri (*const T, *mut T) - C/C++ uyumluluğu için gerekebilir
// resolvedSemanticType TypeAST'ten miras alınır (PointerType*).
 struct PointerTypeAST : public TypeAST {
     std::unique_ptr<TypeAST> pointeeType;
     bool isMutable; // *mut mu, *const mu?
 };

// Tuple Tipleri ((T1, T2, ...))
// resolvedSemanticType TypeAST'ten miras alınır (TupleType*).
 struct TupleTypeAST : public TypeAST {
     std::vector<std::unique_ptr<TypeAST>> elementTypes; // Eleman tipi AST düğümleri
 };


#endif // CNT_COMPILER_TYPES_H
