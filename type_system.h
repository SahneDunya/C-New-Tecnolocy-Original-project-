#ifndef CNT_COMPILER_TYPE_SYSTEM_H
#define CNT_COMPILER_TYPE_SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// İleri bildirimler
struct StructDeclAST; // Struct bildirimi AST düğümüne referans tutabilir
struct EnumDeclAST; // Enum bildirimi AST düğümüne referans tutabilir

// Temel Semantik Tip sınıfı
struct Type {
    enum TypeID {
        VOID_TYPE,    // void (fonksiyon dönüş tipi vb.)
        INT_TYPE,     // int
        FLOAT_TYPE,   // float
        BOOL_TYPE,    // bool
        STRING_TYPE,  // string
        CHAR_TYPE,    // char
        STRUCT_TYPE,  // struct
        ENUM_TYPE,    // enum
        ARRAY_TYPE,   // array
        REFERENCE_TYPE, // &T, &mut T
        POINTER_TYPE, // *const T, *mut T (C/C++ uyumluluğu için)
        FUNCTION_TYPE,// Fonksiyon tipi (imzası)
        ERROR_TYPE    // Anlamsal hata sonucu belirlenemeyen tip
        // ... diğer temel veya kullanıcı tanımlı tipler
    };

    TypeID id;

    Type(TypeID type_id) : id(type_id) {}
    virtual ~Type() = default;

    // Tipleri karşılaştırmak için sanal metod
    virtual bool isEqualTo(const Type* other) const;
    virtual bool isAssignableTo(const Type* targetType, bool isTargetMutable) const; // Bu tip, hedef tipe atanabilir mi?

    // Tipin string temsilini döndüren sanal metod (Hata ayıklama için)
    virtual std::string toString() const = 0;
};

// Somut Tip Sınıfları (Type'tan miras alır)
struct IntType : public Type {
    IntType() : Type(INT_TYPE) {}
    std::string toString() const override { return "int"; }
    bool isEqualTo(const Type* other) const override;
    bool isAssignableTo(const Type* targetType, bool isTargetMutable) const override;
};
// ... Diğer temel tipler için benzer sınıflar (FloatType, BoolType, StringType, CharType, VoidType, ErrorType)

struct StructType : public Type {
    StructDeclAST* declaration; // Tanımlandığı AST düğümüne pointer
    // Struct alanları ve tipleri burada veya SymbolTable'da tutulabilir.
     std::unordered_map<std::string, Type*> fields;

    StructType(StructDeclAST* decl) : Type(STRUCT_TYPE), declaration(decl) {}
     std::string toString() const override; // Implementasyon .cpp'de olacak
     bool isEqualTo(const Type* other) const override;
     bool isAssignableTo(const Type* targetType, bool isTargetMutable) const override;
};
// ... EnumType, ArrayType, ReferenceType, PointerType, FunctionType sınıfları

// Tip Sistemi sınıfı (Tüm tip örneklerini yönetir, aynı tip için tek instance döndürür)
class TypeSystem {
private:
    std::unordered_map<TypeID, std::unique_ptr<Type>> builtinTypes;
    // Kullanıcı tanımlı tipler (struct, enum) için haritalar
    std::unordered_map<std::string, std::unique_ptr<StructType>> structTypes;
    std::unordered_map<std::string, std::unique_ptr<EnumType>> enumTypes; // EnumType sınıfınız olmalı
    // Diğer karmaşık tipler için cache'ler (ReferenceType, ArrayType, FunctionType)
     std::unordered_map<std::string, std::unique_ptr<ReferenceType>> refTypes; // string temsili key olabilir

public:
    TypeSystem(); // Kurucu temel tipleri oluşturur

    Type* getIntType();
    Type* getFloatType();
    Type* getBoolType();
    Type* getStringType();
    Type* getCharType();
    Type* getVoidType();
    Type* getErrorType();

    // Kullanıcı tanımlı tipleri kaydet ve al
    StructType* registerStructType(StructDeclAST* decl);
    StructType* getStructType(const std::string& name);

    // Referans tipi al (&T veya &mut T)
    ReferenceType* getReferenceType(Type* baseType, bool isMutable);

    // Dizi tipi al ([T] veya [T; size] eğer destekleniyorsa)
    ArrayType* getArrayType(Type* elementType); // ArrayType sınıfınız olmalı

    // Tipler arası uyumluluk kontrol metodları
    bool areTypesEqual(const Type* t1, const Type* t2) const;
    bool isAssignable(const Type* valueType, const Type* targetType, bool isTargetMutable) const;
    bool isCoercible(const Type* fromType, const Type* toType) const; // Otomatik dönüşüm

    // İki tipin en yakın ortak atasını bul (Örn: i32 ve f32'nin ortak atası sayı olabilir)
     Type* getCommonAncestor(Type* t1, Type* t2);
};

#endif // CNT_COMPILER_TYPE_SYSTEM_H
