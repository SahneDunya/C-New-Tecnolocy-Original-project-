#include "interface_extractor.h"
#include <fstream>    // Dosya yazma için
#include <sstream>    // String stream için
#include <typeinfo>   // dynamic_cast için typeid
#include "utils.h"    // String yardımcıları için

// Semantic sistemden ihtiyaç duyulan başlıklar
// SymbolInfo ve Type yapılarının tanımları burada görülmeli veya kendi başlıklarından gelmeli.
 #include "symbol_table_impl.h" // Eğer SymbolInfo detayları ayrı dosyadaysa
 #include "type_system_impl.h"  // Eğer Type detayları ayrı dosyadaysa


// Interface Extractor Kurucu
InterfaceExtractor::InterfaceExtractor(Diagnostics& diag, TypeSystem& ts /*, SymbolTable& st*/)
    : diagnostics(diag), typeSystem(ts) /*, symbolTable(st)*/ {
    // Kurulum işlemleri
}

// Semantik analizi yapılmış bir ProgramAST'ten public arayüzü çıkarır
std::shared_ptr<ModuleInterface> InterfaceExtractor::extract(ProgramAST* program, const std::string& moduleName, const std::filesystem::path& sourceFilePath) {
    if (!program) {
        diagnostics.reportInternalError("Arayüz çıkarımı için null ProgramAST.");
        return nullptr;
    }

    // ModuleInterface objesini oluştur
    auto moduleInterface = std::make_shared<ModuleInterface>(moduleName, sourceFilePath.string());

    // AST üzerinde gezerek public bildirimleri çıkar
    // ProgramAST'in çocukları DeclarationAST türündedir. extractPublicItems recursive olacak.
    extractPublicItems(program, *moduleInterface);


    if (moduleInterface->publicSymbols.empty()) {
        diagnostics.reportInfo(program->location,
                               "Modül '" + moduleName + "' public arayüz içermiyor.");
    }

    return moduleInterface;
}

// AST üzerinde gezerek public bildirimleri bulur ve arayüze ekler
void InterfaceExtractor::extractPublicItems(const ASTNode* node, ModuleInterface& interface) {
    if (!node) return;

    // Public olup olmadığını kontrol et (Parser tarafından set edilen isPublic flag'ine bakar)
    // SEMA, isPublic flag'inin geçerli türlerde olup olmadığını kontrol etti.
    bool nodeIsPublic = isPublic(node); // Bildirim düğümleri için isPublic bakar

    // Düğüm bir bildirim türü mü?
    if (const DeclarationAST* decl = dynamic_cast<const DeclarationAST*>(node)) {
        if (nodeIsPublic) {
            // Public olan bir bildirim bulduk. Semantik bilgilerini kullanarak arayüz SymbolInfo'sunu oluştur.
            if (std::shared_ptr<SymbolInfo> interfaceSymbol = buildInterfaceSymbolInfo(decl)) {
                 // SymbolInfo'yu arayüze ekle (Sembol ismine göre haritalanır)
                 interface.addPublicSymbol(interfaceSymbol->name, interfaceSymbol);
                  diagnostics.reportInfo(decl->location, "Public sembol çıkarıldı: " + interfaceSymbol->name); // Debug
            } else {
                // buildInterfaceSymbolInfo hata raporladı.
            }
        }
    }

    // Bu düğüm public olmasa bile, içindeki üyeler public olabilir (örn: struct alanları, impl bloğu metotları)
    // Bu nedenle tüm çocuk düğümleri gezmeye devam etmeliyiz.

    // Çocuk düğümleri gez (recursive)
    // ProgramAST için çocukları 'declarations' vectorüdür.
    if (const ProgramAST* program = dynamic_cast<const ProgramAST*>(node)) {
        for (const auto& decl_ptr : program->declarations) {
            extractPublicItems(decl_ptr.get(), interface); // Bildirimleri gez
        }
    }
    // StructDeclAST içinde alanlar public olabilir (eğer dil kuralları izin veriyorsa ve AST'de public flag varsa)
    else if (const StructDeclAST* structDecl = dynamic_cast<const StructDeclAST*>(node)) {
        // Struct'ın kendisi public ise, alanları da public olabilir (veya alanların kendi public flag'i olabilir)
        // Eğer alanların kendi public flag'i varsa, her alan için extractPublicItems çağrılır.
        // Eğer alanların kendi flag'i yoksa ve struct public ise tüm alanlar public kabul ediliyorsa:
        if (structDecl->isPublic) { // Struct public ise alanları da potansiyel public
            for (const auto& field_ptr : structDecl->fields) {
                // Alanın kendi public flag'i var mı? structFieldAST'te isPublic üyesi varsayalım
                 if (field_ptr->isPublic) {
                     if (auto fieldSymbol = buildInterfaceSymbolInfo(field_ptr.get())) { // structFieldAST DeclarationAST değil, SymbolInfo* doğrudan alınmalı
                          interface.addPublicSymbol(fieldSymbol->name, fieldSymbol);
                     }
                 }
                // Veya sadece alanların public olup olmadığına karar verip, SymbolInfo'larını doğrudan structDecl->resolvedSemanticType (StructType) içinden al.
                // Bu yaklaşıma göre alanların AST düğümlerine isPublic flag'i eklemeye gerek kalmazdı.
                // Şimdiki modelde alanların AST düğümlerine isPublic eklediğimizi varsayalım:
                 extractPublicItems(field_ptr.get(), interface); // StructFieldAST'i gez
            }
        }
    }
    // ImplBlockAST içinde metotlar public olabilir
     else if (const ImplBlockAST* implBlock = dynamic_cast<const ImplBlockAST*>(node)) {
    //     // Impl bloğu kendisi public değil, içindeki metotlar olabilir.
         for (const auto& method_ptr : implBlock->methodsAndAssociatedFunctions) {
              extractPublicItems(method_ptr.get(), interface); // FunctionDeclAST'i gez (methodlar FunctionDeclAST'tir)
         }
     }
    // EnumDeclAST içinde varyantlar public olabilir (genellikle enum public ise varyantları da public kabul edilir)
     else if (const EnumDeclAST* enumDecl = dynamic_cast<const EnumDeclAST*>(node)) {
         if (enumDecl->isPublic) {
              for (const auto& variant_ptr : enumDecl->variants) {
    //               // EnumVaryantları için de isPublic flag varsa kontrol et
                    if(variant_ptr->isPublic) { ... }
    //               // Veya sadece varyantların SymbolInfo'larını EnumDecl->resolvedSemanticType (EnumType) içinden al.
                   extractPublicItems(variant_ptr.get(), interface); // EnumVariantAST'i gez
              }
         }
     }

    // ExpressionAST veya StatementAST gibi diğer düğümlerin çocukları genellikle arayüzün bir parçası olmaz.
    // Örneğin, bir fonksiyon gövdesinin içindeki değişkenler veya ifadeler public değildir.
    // Ancak, nested bildirime izin veriliyorsa (Rust'ta yok), bu mantık değişebilir.
    // Şu anki modelde sadece top-level bildirimleri ve struct/impl/enum içindeki public üyeleri geziyoruz.

    // Eğer bir düğüm türünün özel bir gezinti mantığı yoksa, burada genel bir recursive çağrı yapılabilir.
    // Ancak, sadece public bildirimleri ve onların public alt öğelerini çıkarmak istediğimiz için,
    // tüm AST ağacını körlemesine gezmek yerine ilgili bildirim türlerine odaklanmak daha iyidir.
    // Bu recursive gezinme mantığı, hangi düğüm türlerinin çocuklarının public arayüzde yer alabileceğine bağlı olarak değişir.
    // Yukarıdaki örnekte ProgramAST ve StructDeclAST'in çocuklarını gezme mantığı eklenmiştir.
}


// Bir bildirimin public olup olmadığını belirler (AST'deki isPublic bayrağına bakar)
bool InterfaceExtractor::isPublic(const ASTNode* node) const {
    // Parser'ın set ettiği isPublic bayrağına bakarız.
    // SEMA'nın bu flag'in geçerliliğini kontrol ettiğini varsayarız.
    if (const DeclarationAST* decl = dynamic_cast<const DeclarationAST*>(node)) {
        return decl->isPublic;
    }
    // StructFieldAST veya EnumVariantAST gibi DeclarationAST olmayan ama public olabilecek düğümler için de kontrol ekleyin
     if (const StructFieldAST* field = dynamic_cast<const StructFieldAST*>(node)) {
         return field->isPublic; // Eğer StructFieldAST'te isPublic varsa
     }
     if (const EnumVariantAST* variant = dynamic_cast<const EnumVariantAST*>(node)) {
         return variant->isPublic; // Eğer EnumVariantAST'te isPublic varsa
     }

    return false; // Declaration veya public alt üye türü değilse public değildir.
}

// Public bir bildirimin semantik bilgilerini (SymbolInfo, Type) kullanarak
// ModuleInterface'e eklenecek SymbolInfo objesini oluşturur.
std::shared_ptr<SymbolInfo> InterfaceExtractor::buildInterfaceSymbolInfo(const DeclarationAST* publicDecl) {
    if (!publicDecl) return nullptr;

    // SEMA tarafından AST düğümüne eklenen çözülmüş sembol bilgisini al
    // Bu SymbolInfo, arayüz için gerekli tüm bilgilere (tip, isim, kind, mutability) sahip olmalıdır.
    SymbolInfo* sourceSymbol = publicDecl->resolvedSymbol; // SEMA bunu doldurdu

    if (!sourceSymbol || !sourceSymbol->type) {
        // SEMA'da hata oluştu veya sembol çözülemedi, arayüze eklemeye çalışma.
        // Hata zaten SEMA tarafından raporlandı.
        return nullptr;
    }

    // Arayüz için yeni bir SymbolInfo objesi oluştur.
    // Orijinal SymbolInfo'nun bir kopyasını oluşturmak (deep copy dikkatli yapılmalı)
    // veya arayüz için SymbolInfo yapısını basitleştirerek gerekli alanları kopyalamak gerekir.
    // Önemli: Orijinal AST düğümüne pointer tutmamalıdır (declarationNode = nullptr).

    // Varsayalım SymbolInfo yapısı, arayüz için yeterli bilgiyi (name, type, kind, isMutable) doğrudan tutuyor.
    // Type* pointer'ı semantik Type* objelerine işaret eder, bunlar Tip Sistemi tarafından yönetilir.
    auto interfaceSymbol = std::make_shared<SymbolInfo>(
        sourceSymbol->name,
        sourceSymbol->type, // Semantik Type* pointer'ını kopyala (TypeSystem tarafından yönetilir)
        nullptr,            // DeclarationAST pointer'ı null
        sourceSymbol->isMutable
    );
    interfaceSymbol->kind = sourceSymbol->kind; // Sembol türünü kopyala

    // Eğer SymbolInfo, fonksiyonlar için parametre isimleri gibi ek detaylar tutuyorsa, onları da kopyalayın.
    // Örneğin, eğer SymbolInfo::FunctionKindSymbolInfo gibi kalıtımlı yapılar varsa, ona göre işlem yapın.

    // Example for FunctionKind:
     if (sourceSymbol->kind == SymbolInfo::FUNCTION_KIND) {
         FunctionType* funcType = static_cast<FunctionType*>(sourceSymbol->type);
    //     // Belki SymbolInfo içinde parametre isimlerini tutuyorsunuz?
          shared_ptr<SymbolInfo> interfaceSymbol = std::make_shared<SymbolInfo>(...);
          static_cast<FunctionKindSymbolInfo*>(interfaceSymbol.get())->parameterNames = ... // kopyala
     }

     // Diğer SymbolInfo türleri için de benzer mantık.

    return interfaceSymbol; // Arayüz için SymbolInfo'yu döndür
}


// Çıkarılan ModuleInterface objesini .hnt dosyasına kaydeder (Serialize)
bool InterfaceExtractor::save(const ModuleInterface& interface, const std::filesystem::path& outputPath) {
    // Çıktı dizininin var olduğundan emin ol
    std::error_code ec;
    if (!std::filesystem::create_directories(outputPath.parent_path(), ec)) {
        if (ec && ec.value() != std::errc::file_exists) {
             diagnostics.reportError("", 0, 0, "Modül arayüz çıktı dizini oluşturulamadı: " + outputPath.string() + " - " + ec.message());
             return false;
        }
    }

    // .hnt dosyasını yazma modunda aç
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        diagnostics.reportError("", 0, 0, "Modül arayüz dosyası yazılamadı: " + outputPath.string());
        return false;
    }

    // Dosya formatının başlığını yaz
    file << "// CNT Module Interface File" << std::endl;
    file << "// Module: " << interface.canonicalPath << std::endl;
    file << "// Source: " << interface.filePath << std::endl;
    // Format sürümünü de yazmak faydalı olabilir.
     file << "// Format Version: 1.0" << std::endl;
    file << std::endl;

    // Public sembolleri dosyaya yaz
    if (interface.publicSymbols.empty()) {
        file << "// No public symbols exported." << std::endl;
    } else {
        file << "// Exported Public Symbols:" << std::endl;
        // Public sembolleri isme göre sıralamak çıktıyı daha okunaklı yapar.
        std::vector<std::string> sortedNames;
        for(const auto& pair : interface.publicSymbols) sortedNames.push_back(pair.first);
        std::sort(sortedNames.begin(), sortedNames.end());

        for (const auto& name : sortedNames) {
             const auto& symbol = *interface.publicSymbols.at(name);
            // Sembolü .hnt formatına göre serialize et ve yaz
            file << serializeSymbolInfo(symbol) << std::endl;
        }
    }

    // Eğer C/C++ deklarasyonları da .hnt içinde saklanacaksa, onların da serialization mantığı buraya eklenecek.
    // Örneğin:
     file << std::endl << "// C/C++ Compatible Declarations:" << std::endl;
    // ... serialize C/C++ declarations ...

    file.close();
     diagnostics.reportInfo("", 0, 0, "Modül arayüz dosyası kaydedildi: " + outputPath.string()); // main.cpp raporluyor artık
    return true;
}

// Sembol bilgilerini .hnt formatına serialize eder (string olarak)
// **ÖNEMLİ:** Bu implementasyon, .hnt dosya formatınıza bağlıdır!
std::string InterfaceExtractor::serializeSymbolInfo(const SymbolInfo& symbol) const {
    std::stringstream ss;

    // public belirtecini ekle
    ss << "pub ";

    // Sembol türüne göre formatlı yaz
    // SymbolInfo yapınızda bir 'kind' veya enum üyesi olduğunu varsayarız.
    switch (symbol.kind) {
        case SymbolInfo::FUNCTION_KIND: {
             ss << "fn " << symbol.name;
             // Fonksiyon tipini serialize et (parametreler ve dönüş tipi)
             // SymbolInfo'nun type üyesi FunctionType* olmalı.
              FunctionType* funcType = static_cast<FunctionType*>(symbol.type);
              ss << "(";
             // // Parametre tiplerini ve isimlerini serialize et (Eğer SymbolInfo'da isimleri tutuyorsanız)
              for (size_t i = 0; i < funcType->parameterTypes.size(); ++i) {
                   ss << (funcType->parameterNames[i] + ": ") << serializeType(funcType->parameterTypes[i]); // İsimleri de tutuyorsanız
                   ss << serializeType(funcType->parameterTypes[i]); // Sadece tipleri
                  if (i < funcType->parameterTypes.size() - 1) ss << ", ";
              }
              ss << ") -> " << serializeType(funcType->returnType);
              ss << ";";
             ss << "(...) -> " << serializeType(symbol.type) << ";"; // Placeholder
            break;
        }
        case SymbolInfo::STRUCT_KIND: {
            ss << "struct " << symbol.name;
            // Struct yapısını serialize et (alanlar ve tipleri)
            // SymbolInfo'nun type üyesi StructType* olmalı.
             StructType* structType = static_cast<StructType*>(symbol.type);
             ss << " { ";
            // // Alanları serialize et (isimleri ve tipleri)
             for (const auto& field_pair : structType->fields) { // Eğer alan bilgisi Type'ta tutuluyorsa
                 ss << field_pair.first << ": " << serializeType(field_pair.second) << ", ";
             }
             ss << "}"; // Son virgül olabilir veya olmayabilir, formata göre.
             ss << " { ... };"; // Placeholder
            break;
        }
        case SymbolInfo::ENUM_KIND: {
             ss << "enum " << symbol.name;
             // Enum yapısını serialize et (varyantlar ve ilişkili tipleri varsa)
             // SymbolInfo'nun type üyesi EnumType* olmalı.
              EnumType* enumType = static_cast<EnumType*>(symbol.type);
              ss << " { ";
              for (size_t i = 0; i < enumType->variants.size(); ++i) { // Varyantlar Type'ta tutuluyorsa
                  ss << enumType->variants[i]->name;
             //     // İlişkili tipler varsa serialize et: (Type1, Type2)
                   if (!enumType->variants[i]->associatedTypes.empty()) { ... }
                  if (i < enumType->variants.size() - 1) ss << ", ";
              }
              ss << "}"; // Son virgül olabilir veya olmayabilir.
              ss << " { ... };"; // Placeholder
             break;
        }
        case SymbolInfo::VAR_KIND: { // Global public değişken
             ss << (symbol.isMutable ? "mut " : "let ") << symbol.name << ": " << serializeType(symbol.type) << ";";
             break;
        }
        // ... Diğer sembol türleri (Trait, AssociatedType vb.)
        default:
             ss << "// ERROR: Unknown symbol kind for '" << symbol.name << "' at serialization";
             break;
    }

    return ss.str(); // Oluşturulan stringi döndür
}

// Semantik tipi .hnt formatına serialize eder (string olarak)
// **ÖNEMLİ:** Bu implementasyon, .hnt dosya formatınıza bağlıdır!
std::string InterfaceExtractor::serializeType(const Type* type) const {
    if (!type) return "unknown";

    // Tipin ID'sine veya türüne göre formatlama yap
    switch (type->id) {
        case Type::INT_TYPE: return "int";
        case Type::FLOAT_TYPE: return "float";
        case Type::BOOL_TYPE: return "bool";
        case Type::STRING_TYPE: return "string";
        case Type::CHAR_TYPE: return "char";
        case Type::VOID_TYPE: return "void";
        case Type::STRUCT_TYPE: { // Struct adı
             StructType* structType = static_cast<const StructType*>(type);
             return structType->name; // Type objesi ismini tutmalı
             return "Struct<...>"; // Placeholder
        }
        case Type::ENUM_TYPE: { // Enum adı
             EnumType* enumType = static_cast<const EnumType*>(type);
             return enumType->name; // Type objesi ismini tutmalı
             return "Enum<...>"; // Placeholder
        }
        case Type::ARRAY_TYPE: { // [Element] veya [Element; Size]
              ArrayType* arrayType = static_cast<const ArrayType*>(type);
              std::stringstream ss;
              ss << "[" << serializeType(arrayType->elementType) << "]";
              if (arrayType->size >= 0) { // Sabit boyutlu ise
                   ss << "; " << arrayType->size;
              }
              return ss.str();
             return "[...; ...]"; // Placeholder
        }
        case Type::REFERENCE_TYPE: { // &mut? ReferencedType
              ReferenceType* refType = static_cast<const ReferenceType*>(type);
              return (refType->isMutable ? "&mut " : "&") + serializeType(refType->referencedType);
             return "&... "; // Placeholder
        }
        case Type::FUNCTION_TYPE: { // fn(Args) -> Ret
              FunctionType* funcType = static_cast<const FunctionType*>(type);
              std::stringstream ss;
              ss << "fn(";
              for (size_t i = 0; i < funcType->parameterTypes.size(); ++i) {
                  ss << serializeType(funcType->parameterTypes[i]);
                  if (i < funcType->parameterTypes.size() - 1) ss << ", ";
              }
              ss << ") -> " << serializeType(funcType->returnType);
              return ss.str();
             return "fn(...) -> ..."; // Placeholder
        }
         // Pointer, Tuple gibi diğer tipler...
        case Type::ERROR_TYPE: return "error"; // Hata tipini serialize edilebilir
        default: return "unknown_type";
    }
}
