#ifndef CNT_COMPILER_TYPES_H
#define CNT_COMPILER_TYPES_H

#include "ast.h" // Temel AST sınıfları için
#include "expressions.h" // Tip isimleri IdentifierAST olabilir

#include <vector>
#include <memory> // std::unique_ptr için
#include <string>

// Temel Tipler (int, float, bool, string veya kullanıcı tanımlı Struct/Enum adları)
struct BaseTypeAST : public TypeAST {
    std::string name; // "int", "float", "bool", "string", veya Struct/Enum adı
    // Kullanıcı tanımlı tipler için IdentifierAST de kullanılabilir
     std::unique_ptr<IdentifierAST> name_id;

    BaseTypeAST(std::string n, TokenLocation loc) : name(std::move(n)) { location = loc; }
    // BaseTypeAST(std::unique_ptr<IdentifierAST> id, TokenLocation loc) : name_id(std::move(id)) { location = loc; }

    std::string getNodeType() const override { return "BaseTypeAST"; }
};

// Referans Tipleri (&T, &mut T)
struct ReferenceTypeAST : public TypeAST {
    std::unique_ptr<TypeAST> referencedType; // Referansın gösterdiği tip
    bool isMutable; // &mut mu, & mi?

    ReferenceTypeAST(std::unique_ptr<TypeAST> referenced_t, bool mut, TokenLocation loc)
        : referencedType(std::move(referenced_t)), isMutable(mut) { location = loc; }

    std::string getNodeType() const override { return "ReferenceTypeAST"; }
};

// Dizi Tipleri ([T] veya [T; size])
struct ArrayTypeAST : public TypeAST {
    std::unique_ptr<TypeAST> elementType; // Dizi elemanlarının tipi
    // İsteğe bağlı: Sabit boyutlu diziler için boyut ifadesi
     std::unique_ptr<ExpressionAST> size;

    ArrayTypeAST(std::unique_ptr<TypeAST> element_t, TokenLocation loc)
        : elementType(std::move(element_t)) { location = loc; }
    // ArrayTypeAST(std::unique_ptr<TypeAST> element_t, std::unique_ptr<ExpressionAST> s, TokenLocation loc)
    //     : elementType(std::move(element_t)), size(std::move(s)) { location = loc; }


    std::string getNodeType() const override { return "ArrayTypeAST"; }
};

// Pointer Tipleri (*const T, *mut T) - C/C++ uyumluluğu için gerekebilir
 struct PointerTypeAST : public TypeAST {
     std::unique_ptr<TypeAST> pointeeType;
     bool isMutable; // *mut mu, *const mu?
 };

// Tuple Tipleri ((T1, T2, ...))
 struct TupleTypeAST : public TypeAST {
     std::vector<std::unique_ptr<TypeAST>> elementTypes;
 };


#endif // CNT_COMPILER_TYPES_H
