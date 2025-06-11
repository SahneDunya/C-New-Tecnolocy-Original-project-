#include "type_system.h"
#include "utils.h" // String yardımcıları, hata ayıklama

#include <cassert> // assert için
#include <algorithm> // std::equal, std::sort için

namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // TypeID enum değerini string olarak döndüren yardımcı fonksiyon
    std::string getTypeIDString(TypeID id) {
        switch (id) {
            case TypeID::Error: return "Error";
            case TypeID::Void: return "Void";
            case TypeID::Bool: return "Bool";
            case TypeID::Int: return "Int";
            case TypeID::Float: return "Float";
            case TypeID::Char: return "Char";
            case TypeID::String: return "String";
            case TypeID::Struct: return "Struct";
            case TypeID::Enum: return "Enum";
            case TypeID::Function: return "Function";
            case TypeID::Reference: return "Reference";
            case TypeID::Array: return "Array";
            // ... diğer türler
            default: return "UnknownTypeID";
        }
    }

    // --- Somut Tip Alt Sınıflarının Implementasyonları ---

    // StructFieldInfo karşılaştırma operatorü (Header'da inline edildi)

    // Struct Type toString
    std::string StructType::toString() const {
        std::stringstream ss;
        ss << "struct " << name;
        // Alan detaylarını yazdırmak çok uzun olabilir, sadece ismini döndürmek yaygın.
        // Detaylı arayüz stringi için ayrı serialize metodu olabilir.
         ss << " { ";
         for (size_t i = 0; i < fields.size(); ++i) {
             ss << fields[i].name << ": " << fields[i].type->toString();
             if (i < fields.size() - 1) ss << ", ";
         }
         ss << " }";
        return ss.str();
    }

    // Struct Type isEqualTo
    bool StructType::isEqualTo(const Type* other) const {
        if (!other || other->id != TypeID::Struct) return false;
        const StructType* otherStruct = static_cast<const StructType*>(other);
        // Structlar isimlerine ve alan listelerine göre eşit sayılır.
        // Alan listelerinin sırası da önemlidir.
        // Alan sayısı farklıysa eşit değildir.
        if (name != otherStruct->name || fields.size() != otherStruct->fields.size()) return false;

        // Alanları sırayla karşılaştır (StructFieldInfo::operator== kullanılır)
        return std::equal(fields.begin(), fields.end(), otherStruct->fields.begin());
    }

     // Struct Alanı bulma
     const StructFieldInfo* StructType::findField(const std::string& fieldName) const {
         for(const auto& field : fields) {
             if (field.name == fieldName) return &field;
         }
         return nullptr;
     }

      // Struct tanımını doldurma
     void StructType::define(std::vector<StructFieldInfo> f) {
         // Alan listesi sadece bir kere doldurulmalı. Recursive tiplerde dikkatli olunmalı.
         assert(fields.empty() && "Struct type definition already complete.");
         fields = std::move(f);
          isDefinitionComplete = true; // Eğer böyle bir flag varsa
     }


    // Enum Varyant Bilgisi karşılaştırma operatorü (Header'da inline edildi)

    // Enum Type toString
     std::string EnumType::toString() const {
         std::stringstream ss;
         ss << "enum " << name;
         // Varyant detaylarını yazdırmak uzun olabilir, sadece ismini döndürmek yaygın.
          ss << " { ";
          for (size_t i = 0; i < variants.size(); ++i) {
               ss << variants[i].name;
               if (variants[i].associatedType) {
                   ss << "(" << variants[i].associatedType->toString() << ")";
               }
              if (i < variants.size() - 1) ss << ", ";
          }
          ss << " }";
         return ss.str();
     }

    // Enum Type isEqualTo
    bool EnumType::isEqualTo(const Type* other) const {
        if (!other || other->id != TypeID::Enum) return false;
        const EnumType* otherEnum = static_cast<const EnumType*>(other);
        // Enumlar isimlerine ve varyant listelerine göre eşit sayılır.
        // Varyant listelerinin sırası ve ilişkili tipleri önemlidir.
        if (name != otherEnum->name || variants.size() != otherEnum->variants.size()) return false;

        // Varyantları sırayla karşılaştır (EnumVariantInfo::operator== kullanılır)
        return std::equal(variants.begin(), variants.end(), otherEnum->variants.begin());
    }

     // Enum tanımını doldurma
     void EnumType::define(std::vector<EnumVariantInfo> v) {
         assert(variants.empty() && "Enum type definition already complete.");
         variants = std::move(v);
         // isDefinitionComplete = true;
     }

     // Enum Varyantı bulma
     const EnumVariantInfo* EnumType::findVariant(const std::string& variantName) const {
         for(const auto& variant : variants) {
             if (variant.name == variantName) return &variant;
         }
         return nullptr;
     }


    // Function Type toString
    std::string FunctionType::toString() const {
        std::stringstream ss;
        ss << "fn(";
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            ss << parameterTypes[i]->toString();
            if (i < parameterTypes.size() - 1) ss << ", ";
        }
        ss << ") -> " << returnType->toString();
        return ss.str();
    }

    // Function Type isEqualTo
    bool FunctionType::isEqualTo(const Type* other) const {
        if (!other || other->id != TypeID::Function) return false;
        const FunctionType* otherFunc = static_cast<const FunctionType*>(other);
        // Fonksiyonlar dönüş tipleri ve parametre tipleri listesi aynıysa eşittir.
        // Parametre listelerinin sırası önemlidir.
        if (!returnType->isEqualTo(otherFunc->returnType) || parameterTypes.size() != otherFunc->parameterTypes.size()) return false;

        // Parametre tiplerini sırayla karşılaştır
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            if (!parameterTypes[i]->isEqualTo(otherFunc->parameterTypes[i])) return false;
        }
        return true;
    }

    // Reference Type toString
    std::string ReferenceType::toString() const {
        std::stringstream ss;
        if (isMutable) ss << "&mut ";
        else ss << "&";
        ss << referencedType->toString();
        return ss.str();
    }

    // Reference Type isEqualTo
    bool ReferenceType::isEqualTo(const Type* other) const {
        if (!other || other->id != TypeID::Reference) return false;
        const ReferenceType* otherRef = static_cast<const ReferenceType*>(other);
        // Referanslar, gösterdikleri tip ve mutability aynıysa eşittir.
        return isMutable == otherRef->isMutable && referencedType->isEqualTo(otherRef->referencedType);
    }

    // Array Type toString
    std::string ArrayType::toString() const {
        std::stringstream ss;
        ss << "[" << elementType->toString();
        if (size >= 0) {
            ss << "; " << size;
        }
        ss << "]";
        return ss.str();
    }

    // Array Type isEqualTo
    bool ArrayType::isEqualTo(const Type* other) const {
        if (!other || other->id != TypeID::Array) return false;
        const ArrayType* otherArray = static_cast<const ArrayType*>(other);
        // Diziler, eleman tipi ve boyutu aynıysa eşittir.
        return size == otherArray->size && elementType->isEqualTo(otherArray->elementType);
    }

     // Tuple Type toString (Eğer implemente edildiyse)
      std::string TupleType::toString() const { ... }

     // Tuple Type isEqualTo (Eğer implemente edildiyse)
      bool TupleType::isEqualTo(const Type* other) const { ... }


    // --- Type System Management Class Implementasyonu ---

    // TypeSystem Kurucu
    TypeSystem::TypeSystem()
        // Singleton temel tipler zaten üye olarak tanımlı, ekstra işlem gerekmez
    {
        // Gerekirse ek kurulum veya varsayılan tipleri kaydetme yapılabilir.
        // Örneğin, StringType'ın iç temsilini veya temel tiplerin LLVM karşılığını burada hazırlayabilirsiniz.
    }

    // Bir tipin kanonik string temsilini oluşturur (unique map'ler için key olarak kullanılır)
    // Bu, Type::toString metodunun güvenilir ve tekil bir string ürettiğini varsayar.
    std::string getCanonicalTypeKey(const Type* type) {
         if (!type) return "nullptr"; // Hata durumunda veya null için key
         // Eğer toString metodu kanonik bir temsil sağlıyorsa, onu kullanın.
         // Aksi takdirde, type->id + yapısal bilgilerden (pointer adresleri değil, içerik) bir string veya hash oluşturun.
         // Örneğin:
          if (type->id == TypeID::Reference) {
              const ReferenceType* ref = static_cast<const ReferenceType*>(type);
              return "&" + (ref->isMutable ? "mut " : "") + getCanonicalTypeKey(ref->referencedType);
          }
          if (type->id == TypeID::Function) {
              const FunctionType* func = static_cast<const FunctionType*>(type);
              std::stringstream ss; ss << "fn(";
              for(const auto& param : func->parameterTypes) ss << getCanonicalTypeKey(param) << ",";
              ss << ")->" << getCanonicalTypeKey(func->returnType); return ss.str();
          }
         // Struct/Enum için isim yeterli olabilir unique map'lerde.
          return type->toString(); // Varsayılan olarak toString'i key olarak kullanalım.
         // Ancak, Struct/Enum'un toString'i tüm alanları/varyantları içermiyorsa, ismini kullanmak gerekir.
         if (type->id == TypeID::Struct || type->id == TypeID::Enum) {
             // Kullanıcı tanımlı tipler için isimleri key olarak kullanalım
             if (type->id == TypeID::Struct) return "struct " + static_cast<const StructType*>(type)->name;
             if (type->id == TypeID::Enum) return "enum " + static_cast<const EnumType*>(type)->name;
         }
         // Diğer yapısal tipler için toString'i kullanalım (doğru implemente edildiğini varsayarak)
         return type->toString();
    }


    // Karmaşık tipleri almak veya oluşturmak için fabrika metodları (Unique instance garantisi)

    StructType* TypeSystem::getStructType(const std::string& name, const std::vector<StructFieldInfo>& fields) {
        // İsime göre unique StructType'ı bul veya oluştur.
        // Recursive tipler için, önce registerStructType ile isimle oluşturulur, sonra defineStructType ile doldurulur.
        // Bu metod tam tanım ile yeni bir struct tipi oluşturur veya var olanı (aynı isim ve alanlarla) bulur.
        // İsim + alan listesi yapısının kanonik stringini/hash'ini key olarak kullanmak gerekebilir
        // veya isimle lookup yapıp, bulunan tipin alanlarını karşılaştırmak gerekebilir.
        // Şimdilik, isimle lookup yapıp, alanları compare eden bir iskelet sunalım.

        // Önce sadece isme göre var mı bak (ileri bildirim olabilir)
        auto it = uniqueStructTypes.find("struct " + name); // Key olarak "struct İsim" formatı
        if (it != uniqueStructTypes.end()) {
             // İsim zaten var. Eğer alanları da aynıysa, bu instance'ı döndür.
             if (it->second->fields.size() == fields.size() && std::equal(it->second->fields.begin(), it->second->fields.end(), fields.begin())) {
                // Alanlar da aynı, bu unique instance'ı döndür.
                return it->second.get();
             } else {
                // İsim aynı ama alanlar farklı? Bu genellikle bir hatadır (aynı isimde iki farklı struct tanımı).
                // Veya recursive tipin tanımı yapılıyordur.
                // Recursive tip yönetimi burada dikkatli ele alınmalıdır.
                 diagnostics.reportError("Struct definition conflict or recursive type definition issue for '" + name + "'");
                return nullptr; // Hata durumu
             }
             // Eğer sadece isimle kaydetme (register) kullanılıyorsa, burada alan listesinin boş olması beklenir.
             // Tam tanım yapılıyorsa, alan listesi karşılaştırılır.
        }


        // Yeni StructType instance'ı oluştur ve kaydet
        auto newStruct = std::make_shared<StructType>(name);
        newStruct->define(fields); // Alanlarını doldur
        uniqueStructTypes["struct " + name] = newStruct; // İsim bazlı key ile kaydet
        return newStruct.get();
    }

     EnumType* TypeSystem::getEnumType(const std::string& name, const std::vector<EnumVariantInfo>& variants) {
         // Benzer şekilde EnumType unique instance'ı al veya oluştur (İsim ve varyantlara göre).
         auto it = uniqueEnumTypes.find("enum " + name); // Key olarak "enum İsim" formatı
         if (it != uniqueEnumTypes.end()) {
              if (it->second->variants.size() == variants.size() && std::equal(it->second->variants.begin(), it->second->variants.end(), variants.begin())) {
                  return it->second.get();
              } else {
                   // Hata durumu veya recursive tanım
                   return nullptr;
              }
         }

         auto newEnum = std::make_shared<EnumType>(name);
         newEnum->define(variants); // Varyantları doldur
         uniqueEnumTypes["enum " + name] = newEnum; // İsim bazlı key ile kaydet
         return newEnum.get();
     }


    FunctionType* TypeSystem::getFunctionType(Type* returnType, const std::vector<Type*>& parameterTypes) {
        // Dönüş tipi ve parametre tipleri listesine göre unique FunctionType'ı al veya oluştur.
        // Kanonik string (örn: "fn(int,float)->bool") key olarak kullanılabilir.
        auto tempFunc = FunctionType(returnType, parameterTypes); // Geçici FunctionType (toString yapmak için)
        std::string key = getCanonicalTypeKey(&tempFunc); // Kanonik stringi key olarak kullan

        auto it = uniqueFunctionTypes.find(key);
        if (it != uniqueFunctionTypes.end()) {
            return it->second.get(); // Bulunduysa var olanı döndür
        }

        // Yoksa yeni instance oluştur, kaydet ve döndür
        auto newFunc = std::make_shared<FunctionType>(returnType, parameterTypes);
        uniqueFunctionTypes[key] = newFunc; // Key ile kaydet
        return newFunc.get();
    }

    ReferenceType* TypeSystem::getReferenceType(Type* referencedType, bool isMutable) {
        // Gösterdiği tip ve mutability'ye göre unique ReferenceType'ı al veya oluştur.
        auto tempRef = ReferenceType(referencedType, isMutable); // Geçici ReferenceType (toString yapmak için)
        std::string key = getCanonicalTypeKey(&tempRef); // Kanonik stringi key olarak kullan

        auto it = uniqueReferenceTypes.find(key);
        if (it != uniqueReferenceTypes.end()) {
            return it->second.get(); // Bulunduysa var olanı döndür
        }

        // Yoksa yeni instance oluştur, kaydet ve döndür
        auto newRef = std::make_shared<ReferenceType>(referencedType, isMutable);
        uniqueReferenceTypes[key] = newRef; // Key ile kaydet
        return newRef.get();
    }

    ArrayType* TypeSystem::getArrayType(Type* elementType, long long size) {
        // Eleman tipi ve boyutuna göre unique ArrayType'ı al veya oluştur.
        auto tempArray = ArrayType(elementType, size); // Geçici ArrayType (toString yapmak için)
        std::string key = getCanonicalTypeKey(&tempArray); // Kanonik stringi key olarak kullan

        auto it = uniqueArrayTypes.find(key);
        if (it != uniqueArrayTypes.end()) {
            return it->second.get(); // Bulunduysa var olanı döndür
        }

        // Yoksa yeni instance oluştur, kaydet ve döndür
        auto newArray = std::make_shared<ArrayType>(elementType, size);
        uniqueArrayTypes[key] = newArray; // Key ile kaydet
        return newArray.get();
    }

      TupleType* TypeSystem::getTupleType(const std::vector<Type*>& elementTypes) { ... }


    // Kullanıcı tanımlı tipleri (Struct, Enum) Semantik Analiz sırasında kaydetme ve doldurma
    StructType* TypeSystem::registerStructType(const std::string& name) {
        // Sadece ismiyle bir StructType kaydet veya varsa döndür (recursive/ileri bildirim için).
        // Alan listesi boş olacaktır başlangıçta.
        auto it = uniqueStructTypes.find("struct " + name);
        if (it != uniqueStructTypes.end()) {
            return it->second.get(); // Zaten kaydedilmişse döndür
        }

        // Yeni StructType oluştur (alan listesi boş) ve kaydet
        auto newStruct = std::make_shared<StructType>(name);
        uniqueStructTypes["struct " + name] = newStruct; // İsim bazlı key ile kaydet
        return newStruct.get();
    }

    void TypeSystem::defineStructType(StructType* type, const std::vector<StructFieldInfo>& fields) {
        // Daha önce registerStructType ile oluşturulan StructType'ın tanımını doldur.
        assert(type && type->id == TypeID::Struct && "Invalid type provided for definition.");
        type->define(fields); // Alanlarını doldur
    }

     EnumType* TypeSystem::registerEnumType(const std::string& name) {
         // Sadece ismiyle bir EnumType kaydet veya varsa döndür.
         auto it = uniqueEnumTypes.find("enum " + name);
         if (it != uniqueEnumTypes.end()) {
             return it->second.get();
         }

         auto newEnum = std::make_shared<EnumType>(name);
         uniqueEnumTypes["enum " + name] = newEnum; // İsim bazlı key ile kaydet
         return newEnum.get();
     }

    void TypeSystem::defineEnumType(EnumType* type, const std::vector<EnumVariantInfo>& variants) {
        // Daha önce registerEnumType ile oluşturulan EnumType'ın tanımını doldur.
        assert(type && type->id == TypeID::Enum && "Invalid type provided for definition.");
        type->define(variants); // Varyantlarını doldur
    }


    // --- Tip Kontrolü ve Karşılaştırma ---

    // İki tipin yapısal olarak tamamen eşit olup olmadığını kontrol eder (Canonical check)
    bool TypeSystem::areTypesEqual(const Type* t1, const Type* t2) const {
        // Her iki pointer da null ise eşittir (geçerli bir durum olmamalı ama güvenlik).
        if (!t1 && !t2) return true;
        // Biri null diğeri değilse eşit değildir.
        if (!t1 || !t2) return false;
        // Pointerlar aynı instance'a mı işaret ediyor?
        if (t1 == t2) return true;
        // Farklı instance'lar ama yapısal olarak aynı mı? Sanal isEqualTo metodunu kullan.
        return t1->isEqualTo(t2);
    }

    // Bir değerin tipinin (`valueType`), hedef konumun tipine (`targetType`) atanabilir olup olmadığını kontrol eder.
    bool TypeSystem::isAssignable(const Type* valueType, const Type* targetType, bool targetIsMutable) const {
        // Error tipine her şey atanabilir (hatayı yaymak için)
        if (targetType->isErrorType()) return true;
        // Error tipi hiçbir şeye atanamaz (kaynak hata ise)
        if (valueType->isErrorType()) return false;
        // Tipler tamamen aynıysa atanabilir (basit atama, copy semantiği varsayılarak)
        if (areTypesEqual(valueType, targetType)) return true;

        // İmplicit (örtülü) çevirimleri kontrol et (örn: int -> float)
        if (isCoercible(valueType, targetType)) return true;

        // Referans atama kuralları (örn: &mut T -> &T)
        if (valueType->isReferenceType() && targetType->isReferenceType()) {
            const ReferenceType* valueRef = static_cast<const ReferenceType*>(valueType);
            const ReferenceType* targetRef = static_cast<const ReferenceType*>(targetType);
            // İki referansın gösterdiği tipler aynıysa ve:
            // - Immutable ref (&T) -> Immutable ref (&T) : OK
            // - Mutable ref (&mut T) -> Immutable ref (&T) : OK (immutable reborrow)
            // - Immutable ref (&T) -> Mutable ref (&mut T) : HATA
            // - Mutable ref (&mut T) -> Mutable ref (&mut T) : OK (mutable reborrow)
             if (areTypesEqual(valueRef->referencedType, targetRef->referencedType)) {
                 if (!valueRef->isMutable && targetRef->isMutable) return false; // &T -> &mut T atanamaz
                 // Diğer tüm durumlar (aynı mutability, veya &mut T -> &T) atanabilir varsayalım (OwnershipChecker detayları kontrol eder)
                 return true;
             }
        }

        // Pointer atama kuralları (C/C++ uyumluluğu gerekiyorsa)
         if (valueType->isPointerType() && targetType->isPointerType()) { ... }

        // Struct/Enum/Array tipleri için atama (genellikle sadece tam eşitlik veya move/copy semantiği)
        // Özel durumlar (örn: alt tiplerin üst tipe atanması - eğer miras varsa)

        // Kopyalanamayan (non-copy) tipler için atama bir move işlemidir. Bu her zaman geçerli olmayabilir
        // (örn: move edilmiş değeri atamak). Sahiplik kontrolü (OwnershipChecker) bunu denetler.
        // Burada sadece tip uyumluluğuna bakıyoruz.

        // Mutability kuralı: Mutable olmayan bir konuma (targetIsMutable false) mutable bir değer veya referans atanamaz.
        // Sadece tip uyumluluğu (areTypesEqual veya isCoercible) yeterli olmayabilir.
        // Eğer valueType kendisi mutable bir container (örn: &mut [T] veya mut Struct) ise,
        // ve targetIsMutable false ise, bu atama hata olabilir.
        // Ancak bu kural genellikle SEMA içinde atama ifadesi (`analyzeAssignment`) sırasında kontrol edilir,
        // burada `isAssignable` içinde karmaşıklığı artırmamak için sade tutulabilir.
        // Basitçe, `targetIsMutable` parametresi, targetType'ın kendisinin mutable olup olmadığını kontrol etmek için kullanılır,
        // atama kurallarını karmaşıklaştırmak için değil.


        // Hiçbir uyumluluk kuralı eşleşmedi.
        return false;
    }

    // Bir tipin başka bir tipe örtülü olarak çevrilip çevrilemeyeceğini kontrol eder (örn: int -> float)
    bool TypeSystem::isCoercible(const Type* valueType, const Type* targetType) const {
        // Örneğin:
        // int -> float : true
        // float -> int : false (genellikle explicit casting gerekir)
        // T -> &T : true (Referans alma, l-value ise) - Bu genellikle atama değil, tekli operatör (&) durumudur.
        // &mut T -> &T : true (Referans çevrimi)
        // Array [T; size] -> Slice [T] : true
        // Function pointer -> raw pointer (C/C++ uyumluluğu)
        // Enum Variant -> Enum Type (Match pattern'ları veya constructor gibi)
        // Struct Literal -> Struct Type
        // Match arm result types -> Common Ancestor
        // Bu metod, dilinizdeki tüm otomatik çevirim kurallarını içermelidir.
        if (!valueType || !targetType) return false;

        // Örneğin int'ten float'a çevirim
        if (valueType->isIntType() && targetType->isFloatType()) return true;
        // Örneğin &mut T'den &T'ye çevirim (ReferenceType implementasyonunda da yapılabilir)
        if (valueType->isReferenceType() && targetType->isReferenceType()) {
            const ReferenceType* valueRef = static_cast<const ReferenceType*>(valueType);
            const ReferenceType* targetRef = static_cast<const ReferenceType*>(targetType);
            if (valueRef->isMutable && !targetRef->isMutable && areTypesEqual(valueRef->referencedType, targetRef->referencedType)) {
                return true; // &mut T -> &T çevirimi mümkün
            }
        }

        // ... diğer çevirim kuralları ...

        return false; // Örtülü çevirim yok
    }

    // İki tipin ortak ata tipini bulur (Match ifadesi sonuçları vb. için)
    Type* TypeSystem::getCommonAncestor(const Type* t1, const Type* t2) const {
        // Bu çok karmaşık bir konudur ve dilinizin tip hiyerarşisine bağlıdır (eğer varsa).
        // Temel tipler için (int, float) en üst ata sayısal tip olabilir (eğer varsa).
        // structlar için miras varsa, en yakın ortak üst sınıf.
        // Enum varyantları için enum tipi.
        // Eğer iki tip tamamen uyumsuzsa (örn: int ve struct), ortak ata null olabilir.
        // Basit bir implementasyon: Eğer tiplerden biri diğerine atanabiliyorsa (isAssignable),
        // atanabilen tip ortak ata olabilir. Yoksa null.

        if (areTypesEqual(t1, t2)) return const_cast<Type*>(t1); // Zaten aynılar

        // Eğer t1, t2'ye atanabiliyorsa, t2 bir ata tip olabilir.
        if (isAssignable(t1, t2, false)) return const_cast<Type*>(t2); // mutable önemli değil ata tip için?

        // Eğer t2, t1'e atanabiliyorsa, t1 bir ata tip olabilir.
        if (isAssignable(t2, t1, false)) return const_cast<Type*>(t1);

        // Daha karmaşık hiyerarşi araması gerekebilir.
        // Şimdilik basitçe uyumlu değillerse null döndürelim.

        return nullptr; // Ortak ata bulunamadı (veya uyumsuz tipler)
    }


    // --- Serileştirme / Deserializasyon Yardımcıları Implementasyonu ---

    // Bir Type* objesini .hnt formatına uygun stringe çevirir (serialization)
    std::string TypeSystem::serializeType(const Type* type) const {
        // Bu metod, Type::toString metodunu çağırmalı veya özel bir formatlama yapmalıdır.
        // Eğer toString metodu arayüz dosyasında olması gereken tam ve ayrıştırılabilir formatı veriyorsa,
        // onu kullanmak yeterlidir.
        if (!type) return "null_type";
        return type->toString(); // Type::toString'in doğru formatı ürettiğini varsayalım.
    }

    // .hnt'den okunan tip stringini (serialization formatında) semantik Type* objesine dönüştürür (deserialization)
    // Bu metod, Tip Sistemini kullanarak doğru unique Type* objesini bulmalı veya oluşturmalıdır.
    // Bunun için .hnt'deki tip sözdizimini ayrıştıran basit bir parser gerekir.
    Type* TypeSystem::deserializeType(const std::string& typeString) {
        // Bu, typeString'i ayrıştıracak bir parse işlemi gerektirir.
        // Örneğin "fn(int, string) -> bool" gibi bir stringi okuyup
        // "fn", "(", "int", ",", "string", ")", "->", "bool" tokenlarına ayırıp
        // sonra bu tokenları kullanarak Tip Sisteminin get...Type metodlarını çağırmanız gerekir.
        // Örneğin:
        // Eğer string "int" ise, getIntType() döndür.
        // Eğer string "struct MyStruct" ise, uniqueStructTypes map'inde "struct MyStruct" key'ini ara, bulamazsan hata.
        // Eğer string "fn(int, float) -> bool" ise, önce "int", "float", "bool" stringlerini recursive olarak deserializeType ile çöz,
        // sonra getFunctionType(bool_type, {int_type, float_type}) çağır.
        // Recursive tipler (Struct A { field: B }, Struct B { field: A }) burada dikkatli yönetilmelidir (register/define pattern'ı gibi).

        // Bu placeholder implementasyon sadece basit tipleri handle eder.
        if (typeString == "int") return getIntType();
        if (typeString == "float") return getFloatType();
        if (typeString == "bool") return getBoolType();
        if (typeString == "string") return getStringType();
        if (typeString == "char") return getCharType();
        if (typeString == "void") return getVoidType();

        // Struct veya Enum tipleri için isim bazlı lookup
        if (typeString.rfind("struct ", 0) == 0) {
            std::string structName = cnt_compiler::trim(typeString.substr(7)); // "struct " kısmını atla
            auto it = uniqueStructTypes.find("struct " + structName);
            if (it != uniqueStructTypes.end()) {
                // Bulunan struct tipi döndür. Bu, daha önce register veya get ile oluşturulmuş olmalı.
                return it->second.get();
            }
        }
         if (typeString.rfind("enum ", 0) == 0) {
            std::string enumName = cnt_compiler::trim(typeString.substr(5)); // "enum " kısmını atla
            auto it = uniqueEnumTypes.find("enum " + enumName);
            if (it != uniqueEnumTypes.end()) {
                // Bulunan enum tipi döndür.
                return it->second.get();
            }
        }

        // Karmaşık tipler (Reference, Array, Function) için daha detaylı parser gerekir.
        // Örneğin, "&mut int" -> ReferenceType(getIntType(), true)
         "[string; 10]" -> ArrayType(getStringType(), 10)
         "fn() -> void" -> FunctionType(getVoidType(), {})

        // deserializeType içinde bir Lexer/Parser benzeri yapı kullanmak gerekebilir.
        // Bu, ModuleResolver::parseModuleInterfaceFile metodundaki parser mantığından farklıdır.
        // Oradaki parser .hnt dosyasının genel yapısını (pub fn ..., pub struct ...) ayrıştırır,
        // buradaki deserializeType ise o yapı içindeki tip stringlerini ayrıştırır.

        // Tip stringi ayrıştırılamazsa veya çözülemezse hata tipi döndür
         diagnostics.reportError("Unknown type string during deserialization: '" + typeString + "'"); // Diagnostic System'e erişim lazım
        // ModuleResolver bu metodu çağırırken Diagnostic System'e erişim sağlayacaktır.
         return getErrorType(); // Hata durumunda ErrorType döndür
    }


} // namespace cnt_compiler
