#ifndef CNT_COMPILER_TYPE_SYSTEM_H
#define CNT_COMPILER_TYPE_SYSTEM_H

#include <string>
#include <vector>
#include <memory>        // std::unique_ptr, std::shared_ptr için
#include <unordered_map> // Tip instance'larını yönetmek için
#include <sstream>       // toString için
#include <cstddef>       // size_t için

// Forward declarations (Diğer bileşenler TypeSystem'i kullanır, TypeSystem diğerlerini daha az kullanır)
 struct StructDeclAST; // Eğer Type içinde AST pointer'ı tutulacaksa gerekli
 struct EnumDeclAST;

namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // Tip türlerini ayırt etmek için enum
    enum class TypeID {
        Error,        // Anlamsal hata durumunda kullanılan özel tip
        Void,         // () unit tipi, dönüş değeri olmayan fonksiyonlar
        Bool,         // boolean (true/false)
        Int,          // Tamsayı
        Float,        // Ondalıklı sayı
        Char,         // Karakter
        String,       // String dilimi (&str benzeri) veya String objesi
        Struct,       // Kullanıcı tanımlı struct
        Enum,         // Kullanıcı tanımlı enum
        Function,     // Fonksiyon imzası/pointerı
        Reference,    // Referans tipi (&T, &mut T)
        Array,        // Dizi tipi ([T; size] veya [T])
         Pointer,   // Raw pointer tipi (*T, *mut T) - C/C++ uyumluluğu için gerekebilir
         Tuple,     // Tuple tipi (T1, T2, ...)
         Trait,     // Trait object veya tanımı
         Alias      // Tip aliasları
    };

    // TypeID enum değerini string olarak döndüren yardımcı fonksiyon
    std::string getTypeIDString(TypeID id);

    // Tüm anlamsal tiplerin temel sınıfı
    struct Type {
        TypeID id; // Tip türü

        Type(TypeID type_id) : id(type_id) {}
        virtual ~Type() = default; // Polymorphic delete için sanal yıkıcı

        // Tipin string temsilini döndürür (Hata ayıklama, tanı, serileştirme için)
        virtual std::string toString() const = 0;

        // İki tipin yapısal olarak tamamen aynı olup olmadığını kontrol eder (Canonical check)
        // Unique tip instance'larını karşılaştırmak için kullanılır.
        virtual bool isEqualTo(const Type* other) const = 0;

        // Ortak kolaylık sağlayan metodlar
        bool isErrorType() const { return id == TypeID::Error; }
        bool isVoidType() const { return id == TypeID::Void; }
        bool isBoolType() const { return id == TypeID::Bool; }
        bool isIntType() const { return id == TypeID::Int; }
        bool isFloatType() const { return id == TypeID::Float; }
        bool isCharType() const { return id == TypeID::Char; }
        bool isStringType() const { return id == TypeID::String; }
        bool isStructType() const { return id == TypeID::Struct; }
        bool isEnumType() const { return id == TypeID::Enum; }
        bool isFunctionType() const { return id == TypeID::Function; }
        bool isReferenceType() const { return id == TypeID::Reference; }
        bool isArrayType() const { return id == TypeID::Array; }
        // ... diğer is... metodları
    };

    // --- Somut Tip Alt Sınıfları ---

    // Hata Tipi (Singleton)
    struct ErrorType : public Type {
        ErrorType() : Type(TypeID::Error) {}
        std::string toString() const override { return "error"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Error; }
    };

    // Void Tipi (Singleton)
    struct VoidType : public Type {
        VoidType() : Type(TypeID::Void) {}
        std::string toString() const override { return "void"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Void; }
    };

     // Bool Tipi (Singleton)
    struct BoolType : public Type {
        BoolType() : Type(TypeID::Bool) {}
        std::string toString() const override { return "bool"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Bool; }
    };

    // Int Tipi (Singleton)
    struct IntType : public Type {
        IntType() : Type(TypeID::Int) {}
        std::string toString() const override { return "int"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Int; }
    };

     // Float Tipi (Singleton)
    struct FloatType : public Type {
        FloatType() : Type(TypeID::Float) {}
        std::string toString() const override { return "float"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Float; }
    };

     // Char Tipi (Singleton)
    struct CharType : public Type {
        CharType() : Type(TypeID::Char) {}
        std::string toString() const override { return "char"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::Char; }
    };

     // String Tipi (Singleton)
    struct StringType : public Type {
        StringType() : Type(TypeID::String) {}
        std::string toString() const override { return "string"; }
        bool isEqualTo(const Type* other) const override { return other && other->id == TypeID::String; }
    };


    // Struct Alan Bilgisi (StructType içinde kullanılır)
    struct StructFieldInfo {
        std::string name;
        Type* type; // Alanın tipi (TypeSystem tarafından yönetilen Type* objesine pointer)
        size_t offset; // Alanın struct içindeki byte offseti (CodeGen tarafından hesaplanır/kullanılır)
         bool isPublic; // Alanın görünürlüğü (InterfaceExtractor için)

        // Kurucu
        StructFieldInfo(std::string n, Type* t, size_t off = 0 /* offset CodeGen'de hesaplanır */)
            : name(std::move(n)), type(t), offset(off) {}

        // Karşılaştırma için (fieldName ve Type* pointerını karşılaştır)
        bool operator==(const StructFieldInfo& other) const {
            return name == other.name && type == other.type; // Pointer karşılaştırması yeterli (TypeSystem tek instance sağlar)
        }
    };


    // Struct Tipi
    struct StructType : public Type {
        std::string name; // Struct'ın ismi (kullanıcı tanımlı isim)
        std::vector<StructFieldInfo> fields; // Alan listesi

        // recursive tipler için ileri bildirim durumunda bu pointer kullanılabilir.
        // bool isDefinitionComplete = false;

        // Kurucu (Sadece isimle başlatılabilir - ileri bildirim)
        StructType(std::string n) : Type(TypeID::Struct), name(std::move(n)) {}
        // Tam tanım kurucusu veya define metodunu kullanın

        std::string toString() const override; // struct Name { field1: Type1, ... }
        bool isEqualTo(const Type* other) const override; // İsmi ve alanları karşılaştır

        // Struct tanımını doldurmak için (recursive tipler için isimle başlatılıp sonra doldurulur)
        void define(std::vector<StructFieldInfo> f);

        // Alanı isme göre bulur
        const StructFieldInfo* findField(const std::string& fieldName) const;
    };

     // Enum Varyant Bilgisi (EnumType içinde kullanılır)
     struct EnumVariantInfo {
         std::string name;
         Type* associatedType; // İlişkilendirilmiş veri tipi (nullptr veya TupleType* vb.)
         int discriminant; // C-like enum'lar veya LLVM için (CodeGen hesaplar)

         // Kurucu
         EnumVariantInfo(std::string n, Type* assoc_t = nullptr, int disc = 0 /* discriminant CodeGen/SEMA hesaplar */)
             : name(std::move(n)), associatedType(assoc_t), discriminant(disc) {}

          // Karşılaştırma için
         bool operator==(const EnumVariantInfo& other) const {
             return name == other.name && associatedType == other.associatedType; // Pointer karşılaştırması yeterli
         }
     };

    // Enum Tipi
    struct EnumType : public Type {
        std::string name; // Enum'un ismi
        std::vector<EnumVariantInfo> variants; // Varyant listesi

        // recursive tipler için ileri bildirim durumunda bu pointer kullanılabilir.
         bool isDefinitionComplete = false;

        // Kurucu (Sadece isimle başlatılabilir)
        EnumType(std::string n) : Type(TypeID::Enum), name(std::move(n)) {}
        // Tam tanım kurucusu veya define metodunu kullanın.

        std::string toString() const override; // enum Name { Variant1, Variant2(Type), ... }
        bool isEqualTo(const Type* other) const override; // İsmi ve varyantları karşılaştır

         // Enum tanımını doldurmak için
        void define(std::vector<EnumVariantInfo> v);

         // Varyantı isme göre bulur
        const EnumVariantInfo* findVariant(const std::string& variantName) const;
    };


    // Fonksiyon Tipi
    struct FunctionType : public Type {
        Type* returnType;
        std::vector<Type*> parameterTypes;
         std::vector<std::string> parameterNames; // Opsiyonel, sadece isimler arayüzde gösterilebilir

        FunctionType(Type* ret_t, std::vector<Type*> param_ts)
            : Type(TypeID::Function), returnType(ret_t), parameterTypes(std::move(param_ts)) {}
            // Kurucuya parameterNames de eklenebilir

        std::string toString() const override; // fn(T1, T2) -> Ret
        bool isEqualTo(const Type* other) const override; // Dönüş tipini ve parametre tiplerini karşılaştır (pointer karşılaştırması yeterli)
         // Parametre isimleri isEqualTo'ya dahil edilmez (tip eşitliği için isimler önemli değil)
    };

    // Referans Tipi
    struct ReferenceType : public Type {
        Type* referencedType; // Referansın gösterdiği tip
        bool isMutable;      // &mut vs &

        ReferenceType(Type* ref_t, bool mut)
            : Type(TypeID::Reference), referencedType(ref_t), isMutable(mut) {}

        std::string toString() const override; // &T or &mut T
        bool isEqualTo(const Type* other) const override; // Gösterilen tipi ve mutability'yi karşılaştır
    };

    // Dizi Tipi
    struct ArrayType : public Type {
        Type* elementType; // Elemanların tipi
        long long size;      // Dizi boyutu (-1 dynamic size arrays veya slice için)

        ArrayType(Type* elem_t, long long s = -1)
            : Type(TypeID::Array), elementType(elem_t), size(s) {}

        std::string toString() const override; // [T; size] veya [T]
        bool isEqualTo(const Type* other) const override; // Eleman tipini ve boyutu karşılaştır
    };

     // Tuple Tipi (Eğer dil destekliyorsa)
     struct TupleType : public Type {
          std::vector<Type*> elementTypes;
          TupleType(std::vector<Type*> elem_ts) : Type(TypeID::Tuple), elementTypes(std::move(elem_ts)) {}
          std::string toString() const override; // (T1, T2, ...)
          bool isEqualTo(const Type* other) const override;
     };


    // --- Tip Sistemi Yönetim Sınıfı ---

    class TypeSystem {
    private:
        // Singleton temel tip instance'ları (Bellek yönetimi TypeSystem tarafından yapılır)
        ErrorType errorType;
        VoidType voidType;
        BoolType boolType;
        IntType intType;
        FloatType floatType;
        CharType charType;
        StringType stringType;

        // Unique karmaşık tip instance'ları için depolama
        // Unique'liği sağlamak için map kullanmak yaygın (serialize edilmiş string veya hash key olarak)
        // Map yerine vector de kullanılabilir ama arama (get) yavaşlar.
        // shared_ptr kullanmak, Type objelerinin ömrünü TypeSystem'e bağlar.
        std::unordered_map<std::string, std::shared_ptr<StructType>> uniqueStructTypes; // Key: Struct ismi
        std::unordered_map<std::string, std::shared_ptr<EnumType>> uniqueEnumTypes;   // Key: Enum ismi

        // Yapısal tipler (Function, Reference, Array, Tuple). Bunların unique'liği yapısına bağlıdır.
        // Anahtar olarak tipin kanonik string temsilini kullanmak yaygın.
        std::unordered_map<std::string, std::shared_ptr<FunctionType>> uniqueFunctionTypes; // Key: Kanonik string (örn: "fn(int,float)->bool")
        std::unordered_map<std::string, std::shared_ptr<ReferenceType>> uniqueReferenceTypes; // Key: Kanonik string (örn: "&mut int")
        std::unordered_map<std::string, std::shared_ptr<ArrayType>> uniqueArrayTypes;     // Key: Kanonik string (örn: "[int; 10]" veya "[string]")
         std::unordered_map<std::string, std::shared_ptr<TupleType>> uniqueTupleTypes; // Key: Kanonik string (örn: "(int, bool)")


        // Yardımcı metod: Bir tipin kanonik string temsilini oluşturur (unique map'ler için key olarak kullanılır)
        // Bu metod Type::toString'ten farklı olabilir, sadece unique key oluşturmak için kullanılır.
         std::string getCanonicalTypeKey(const Type* type) const; // Bu helper type_system.cpp içinde kalabilir.

    public:
        // Kurucu
        TypeSystem();

        // Singleton temel tip instance'larına erişim
        Type* getErrorType() { return &errorType; }
        Type* getVoidType() { return &voidType; }
        Type* getBoolType() { return &boolType; }
        Type* getIntType() { return &intType; }
        Type* getFloatType() { return &floatType; }
        Type* getCharType() { return &charType; }
        Type* getStringType() { return &stringType; }
        // ... diğer temel tipler

        // Karmaşık tipleri almak veya oluşturmak için fabrika metodları (Unique instance garantisi)
        // Eğer tip varsa pointer'ını döndürür, yoksa oluşturur, kaydeder ve pointer'ını döndürür.
        // Recursive tipleri yönetmek için, önce sadece isimle kaydetme (register), sonra detayları doldurma (define) pattern'ı gerekebilir.
        StructType* getStructType(const std::string& name, const std::vector<StructFieldInfo>& fields); // Adı ve alanları aynı olan StructType'ı al veya oluştur
        EnumType* getEnumType(const std::string& name, const std::vector<EnumVariantInfo>& variants);   // Adı ve varyantları aynı olan EnumType'ı al veya oluştur
        FunctionType* getFunctionType(Type* returnType, const std::vector<Type*>& parameterTypes);      // İmza aynı olan FunctionType'ı al veya oluştur
        ReferenceType* getReferenceType(Type* referencedType, bool isMutable);                         // Gösterdiği tip ve mutability aynı olan ReferansType'ı al veya oluştur
        ArrayType* getArrayType(Type* elementType, long long size = -1);                               // Eleman tipi ve boyutu aynı olan ArrayType'ı al veya oluştur
        // TupleType* getTupleType(const std::vector<Type*>& elementTypes);                              // Eleman tipleri aynı olan TupleType'ı al veya oluştur


        // Kullanıcı tanımlı tipleri (Struct, Enum) Semantik Analiz sırasında kaydetme ve doldurma
        // İleri bildirim ve recursive tipler için kullanılır.
        // Sema'nın ilk geçişinde sadece isim kaydedilir. İkinci geçişte detaylar (alanlar, varyantlar) doldurulur.
        StructType* registerStructType(const std::string& name); // İsimle StructType oluştur (varsa döndür) - Tanım boş
        void defineStructType(StructType* type, const std::vector<StructFieldInfo>& fields); // Daha önce oluşturulan StructType'ın tanımını doldur

        EnumType* registerEnumType(const std::string& name); // İsimle EnumType oluştur (varsa döndür) - Tanım boş
        void defineEnumType(EnumType* type, const std::vector<EnumVariantInfo>& variants); // Daha önce oluşturulan EnumType'ın tanımını doldur

        // İsime göre daha önce kaydedilmiş Struct/Enum tipini bulur (Genellikle Sembol Tablosu kullanır)
        // Bu metod doğrudan kullanılmayabilir, sembol tablosu ismi TipSystem'deki Type*'a mapler.
        // StructType* lookupStructType(const std::string& name);
        // EnumType* lookupEnumType(const std::string& name);


        // --- Tip Kontrolü ve Karşılaştırma ---

        // İki tipin yapısal olarak tamamen eşit olup olmadığını kontrol eder (Canonical check)
        bool areTypesEqual(const Type* t1, const Type* t2) const;

        // Bir değerin tipinin (`valueType`), hedef konumun tipine (`targetType`) atanabilir olup olmadığını kontrol eder.
        // `targetIsMutable`: Hedef konum (değişken, alan, dereference sonucu) mutable mı?
        // Bu metod, atama kurallarını, mutability'yi, referans kurallarını, move/copy semantiğini içerir.
        bool isAssignable(const Type* valueType, const Type* targetType, bool targetIsMutable) const;

        // Bir tipin başka bir tipe örtülü olarak çevrilip çevrilemeyeceğini kontrol eder (örn: int -> float)
        bool isCoercible(const Type* valueType, const Type* targetType) const;

        // İki tipin ortak ata tipini bulur (Match ifadesi sonuçları, if/else dalları vb. için)
        // Uyumlu değillerse nullptr dönebilir.
        Type* getCommonAncestor(const Type* t1, const Type* t2) const;

        // --- Serileştirme / Deserializasyon Yardımcıları (.hnt dosyası için) ---
        // Bu metodlar InterfaceExtractor ve ModuleResolver tarafından kullanılır.

        // Bir Type* objesini .hnt formatına uygun stringe çevirir (serialization)
        // Genellikle Type::toString çağrılır veya özel bir formatlama yapılır.
        std::string serializeType(const Type* type) const;

        // .hnt'den okunan tip stringini (serialization formatında) semantik Type* objesine dönüştürür (deserialization)
        // Bu metod, Tip Sistemini kullanarak doğru unique Type* objesini bulmalı veya oluşturmalıdır.
        // Bunun için .hnt'deki tip sözdizimini ayrıştıran basit bir parser gerekir.
        Type* deserializeType(const std::string& typeString);
    };

} // namespace cnt_compiler

#endif // CNT_COMPILER_TYPE_SYSTEM_H
