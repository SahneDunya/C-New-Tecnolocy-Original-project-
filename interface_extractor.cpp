#include "interface_extractor.h"
#include <fstream>    // Dosya yazma için
#include <sstream>    // String stream için
#include <typeinfo>   // dynamic_cast için typeid

// Semantik sistem implementasyonları (Include edilmez, başlık yeterli)
 #include "symbol_table.cpp"
 #include "type_system.cpp"


// Interface Extractor Kurucu
InterfaceExtractor::InterfaceExtractor(Diagnostics& diag, TypeSystem& ts)
    : diagnostics(diag), typeSystem(ts) {
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
    extractPublicItems(program, *moduleInterface);

    // Eğer arayüz boşsa (hiç public öğe yoksa) ve bu bir hata değilse (izin veriliyorsa),
    // boş arayüz objesini döndürürüz.
    if (moduleInterface->publicSymbols.empty()) {
        diagnostics.reportInfo(program->location.filename, program->location.line, program->location.column,
                               "Modül '" + moduleName + "' public arayüz içermiyor.");
    }

    return moduleInterface;
}

// AST üzerinde gezerek public bildirimleri bulur ve arayüze ekler
void InterfaceExtractor::extractPublicItems(const ASTNode* node, ModuleInterface& interface) {
    if (!node) return;

    // Düğüm public mi kontrol et (kendi mantığınıza göre)
    if (isPublic(node)) {
        // Eğer public ise ve arayüze eklenebilecek türdense
        // SEMA tarafından AST düğümüne eklenmiş sembol/tip bilgilerini al ve arayüz objesi için yeni SymbolInfo oluştur.
        if (std::shared_ptr<SymbolInfo> interfaceSymbol = buildInterfaceSymbolInfo(node)) {
             interface.addPublicSymbol(interfaceSymbol->name, interfaceSymbol);
        }
        // public öğe bulunduğunda, içindeki public üyeleri de recursive olarak kontrol etmek gerekebilir
        // (örn: public bir struct'ın public alanları veya methodları)
    }

    // Çocuk düğümleri gezmeye devam et (recursive)
    // ProgramAST için çocukları 'declarations' vectorüdür.
    if (const ProgramAST* program = dynamic_cast<const ProgramAST*>(node)) {
        for (const auto& decl_ptr : program->declarations) {
            extractPublicItems(decl_ptr.get(), interface); // Bildirimleri gez
        }
    }
    // BlockStatementAST için çocukları 'statements' vectorüdür.
    else if (const BlockStatementAST* block = dynamic_cast<const BlockStatementAST*>(node)) {
         for (const auto& stmt_ptr : block->statements) {
            extractPublicItems(stmt_ptr.get(), interface); // Deyimleri gez (local değişkenler public olamaz ama belki nested blocklar?)
        }
    }
    // Diğer AST düğümlerinin de (struct gövdesi, enum varyantları, impl blokları vb.)
    // gezilmesi ve public üyelerinin kontrol edilmesi gerekebilir.
    // Örneğin FunctionDeclAST içinde public iç öğe olmaz (parametresi/dönüş tipi sembol info'da),
    // ama StructDeclAST içinde alanlar, EnumDeclAST içinde varyantlar public olabilir.
    // ImplBlockAST içinde methodlar public olabilir.

    // Örnek: Struct alanlarını gezmek (eğer struct public ise)
     if (const StructDeclAST* structDecl = dynamic_cast<const StructDeclAST*>(node)) {
         if (isPublic(structDecl)) { // Struct kendisi public ise alanlarını kontrol et
             for (const auto& field_ptr : structDecl->fields) {
                 if (isPublic(field_ptr.get())) { // Alan public mi?
                     if (std::shared_ptr<SymbolInfo> fieldSymbol = buildInterfaceSymbolInfo(field_ptr.get())) {
    //                      // Alanlar genellikle FunctionAST veya TypeAST gibi bir tipe sahip SymbolInfo olmalı
                          interface.addPublicSymbol(fieldSymbol->name, fieldSymbol);
                     }
                 }
             }
         }
     }
    // ... Diğer karmaşık yapılar (Enum varyantları, Impl blokları içindeki methodlar)
}

// Bir bildirimin public olup olmadığını belirler (Örnek: 'pub' anahtar kelimesi var mı?)
bool InterfaceExtractor::isPublic(const ASTNode* node) const {
    // Bu metod, CNT dilinizdeki "public" olma kuralını implemente eder.
    // Örnek: Eğer düğümün başında 'pub' anahtar kelimesi varsa (parser tarafından AST düğümüne bayrak olarak eklenmişse)
     if (node->hasPublicKeywordFlag) return true; // AST düğümlerinde böyle bir bayrak olduğunu varsayalım

    // Varsayılan görünürlük kurallarını da burada uygulayın (örn: modül içinde varsayılan olarak private)
    // Şimdilik basitçe bazı bildirim türlerini varsayılan olarak public kabul edelim (Gerçekte böyle olmaz!)
    if (dynamic_cast<const FunctionDeclAST*>(node) ||
        dynamic_cast<const StructDeclAST*>(node) ||
        dynamic_cast<const EnumDeclAST*>(node) ||
        (dynamic_cast<const VarDeclAST*>(node) && dynamic_cast<const VarDeclAST*>(node)->isGlobal)) { // Global değişkenler
          diagnostics.reportWarning(node->location.filename, node->location.line, node->location.column,
         //                        "Varsayılan public kuralı uygulanıyor. Gerçek implementasyonda 'pub' anahtar kelimesi kullanılmalı.");
         return true; // Placeholder: Fonksiyon, Struct, Enum, Global Var'ları varsayılan public yapalım
    }


    return false; // Varsayılan olarak private
}

// Public bir bildirimin semantik bilgilerini çıkarır ve ModuleInterface'e eklenecek SymbolInfo oluşturur.
// SEMA tarafından AST düğümüne eklenmiş sembol/tip bilgilerini kullanır.
std::shared_ptr<SymbolInfo> InterfaceExtractor::buildInterfaceSymbolInfo(const ASTNode* publicNode) {
    // AST düğümünün türüne göre semantik bilgiyi al
    // SEMA'nın AST düğümlerine eklediği resolvedSymbol veya resolvedSemanticType pointerlarını kullanın.

    // Örnek: FunctionDeclAST için
    if (const FunctionDeclAST* funcDecl = dynamic_cast<const FunctionDeclAST*>(publicNode)) {
        // FunctionDeclAST'in SEMA tarafından resolvedSymbol ve resolvedSemanticType (FunctionType) ile işaretlendiğini varsayalım
         SymbolInfo* sourceSymbol = funcDecl->resolvedSymbol; // Orijinal sembol tablosundaki sembol
         FunctionType* funcType = static_cast<FunctionType*>(funcDecl->resolvedSemanticType);

        // Arayüz objesi için yeni bir SymbolInfo oluştur. Sadece gerekli public bilgileri içerir.
        // Pointerları (örn: declarationNode) orijinal AST'ye değil, null veya ilgili arayüz yapısına işaret etmeli.
         shared_ptr<SymbolInfo> interfaceSymbol = std::make_shared<SymbolInfo>(sourceSymbol->name, sourceSymbol->type, nullptr, sourceSymbol->isMutable);
         interfaceSymbol->kind = sourceSymbol->kind; // Sembol türünü kopyala (FUNCTION_KIND gibi)
         diagnostics.reportWarning(publicNode->location.filename, publicNode->location.line, publicNode->location.column,
                               "buildInterfaceSymbolInfo implementasyonu eksik (Fonksiyon)."); // Placeholder
         return nullptr; // Placeholder

    }
    // Örnek: StructDeclAST için
    else if (const StructDeclAST* structDecl = dynamic_cast<const StructDeclAST*>(publicNode)) {
         // StructDeclAST'in SEMA tarafından resolvedSymbol ve resolvedSemanticType (StructType) ile işaretlendiğini varsayalım
         SymbolInfo* sourceSymbol = structDecl->resolvedSymbol;
         StructType* structType = static_cast<StructType*>(structDecl->resolvedSemanticType);

        // Arayüz SymbolInfo oluştur (Adı, tipi)
         shared_ptr<SymbolInfo> interfaceSymbol = std::make_shared<SymbolInfo>(sourceSymbol->name, sourceSymbol->type, nullptr, sourceSymbol->isMutable);
         interfaceSymbol->kind = sourceSymbol->kind; // STRUCT_KIND

        // StructType içinde public alan bilgisi varsa, onu da kopyalamak gerekebilir.
        // Arayüz StructType nesnesi, orijinal StructType'tan sadece public alanları içermelidir.
        // Bu, Tip Sisteminizde arayüze özel tip temsilleri gerektirebilir.
         diagnostics.reportWarning(publicNode->location.filename, publicNode->location.line, publicNode->location.column,
                               "buildInterfaceSymbolInfo implementasyonu eksik (Struct)."); // Placeholder
         return nullptr; // Placeholder
    }
     // Örnek: VarDeclAST (Global public değişken) için
    else if (const VarDeclAST* varDecl = dynamic_cast<const VarDeclAST*>(publicNode)) {
         // VarDeclAST'in SEMA tarafından resolvedSymbol ve resolvedSemanticType ile işaretlendiğini varsayalım
         SymbolInfo* sourceSymbol = varDecl->resolvedSymbol;
         Type* varType = varDecl->resolvedSemanticType;

        // Arayüz SymbolInfo oluştur (Adı, tipi, mutability)
         shared_ptr<SymbolInfo> interfaceSymbol = std::make_shared<SymbolInfo>(sourceSymbol->name, sourceSymbol->type, nullptr, sourceSymbol->isMutable);
         interfaceSymbol->kind = sourceSymbol->kind; // VAR_KIND
         diagnostics.reportWarning(publicNode->location.filename, publicNode->location.line, publicNode->location.column,
                               "buildInterfaceSymbolInfo implementasyonu eksik (Global Var)."); // Placeholder
         return nullptr; // Placeholder
    }
     // ... Diğer public olabilecek AST düğüm türleri (EnumDecl, TraitDecl, vb.)

    // Desteklenmeyen public düğüm türü (Bu bir SEMA hatası veya isPublic hatası olabilir)
    diagnostics.reportInternalError("Arayüz çıkarımı için desteklenmeyen public bildirim türü: " + std::string(typeid(*publicNode).name()));
    return nullptr;
}


// Çıkarılan ModuleInterface objesini .hnt dosyasına kaydeder (Serialize)
bool InterfaceExtractor::save(const ModuleInterface& interface, const std::filesystem::path& outputPath) {
    // Çıktı dizininin var olduğundan emin ol
    std::error_code ec; // Hata kodu için
    if (!std::filesystem::create_directories(outputPath.parent_path(), ec)) {
        if (ec && ec.value() != std::errc::file_exists) { // Dizin zaten varsa hata değil
             diagnostics.reportError("", 0, 0, "Modül arayüz çıktı dizini oluşturulamadı: " + ec.message());
             return false;
        }
    }


    // .hnt dosyasını yazma modunda aç
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        diagnostics.reportError("", 0, 0, "Modül arayüz dosyası yazılamadı: " + outputPath.string());
        return false;
    }

    // Dosya formatının başlığını yaz (isteğe bağlı)
    file << "// CNT Module Interface File" << std::endl;
    file << "// Module: " << interface.canonicalPath << std::endl;
    file << "// Source: " << interface.filePath << std::endl;
    file << std::endl;

    // Public sembolleri dosyaya yaz
    if (interface.publicSymbols.empty()) {
        file << "// No public symbols exported." << std::endl;
    } else {
        file << "// Exported Public Symbols:" << std::endl;
        for (const auto& pair : interface.publicSymbols) {
            const std::string& name = pair.first;
            const SymbolInfo& symbol = *pair.second;

            // Sembolü .hnt formatına göre serialize et ve yaz
            file << serializeSymbolInfo(symbol) << std::endl;
        }
    }


    file.close();
     diagnostics.reportInfo("", 0, 0, "Modül arayüz dosyası kaydedildi: " + outputPath.string());
    return true;
}

// Sembol bilgilerini .hnt formatına serialize eder (string olarak)
// **ÖNEMLİ:** Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır!
std::string InterfaceExtractor::serializeSymbolInfo(const SymbolInfo& symbol) const {
    std::stringstream ss;
    ss << "pub "; // Public olduğunu belirt

    // Sembol türüne göre formatlı yaz (Fonksiyon, struct, var vb.)
    // Bu, SymbolInfo yapınızda bir 'kind' veya enum üyesi olduğunu varsayar.
     switch (symbol.kind) {
         case SymbolInfo::FUNCTION_KIND:
             ss << "fn " << symbol.name;
             // Fonksiyon tipini serialize et (parametreler, dönüş tipi)
              FunctionType* funcType = static_cast<FunctionType*>(symbol.type);
              ss << "(";
              for (size_t i = 0; i < funcType->parameterTypes.size(); ++i) {
                  ss << serializeType(funcType->parameterTypes[i]);
                  if (i < funcType->parameterTypes.size() - 1) ss << ", ";
              }
              ss << ") -> " << serializeType(funcType->returnType);
              ss << ";";
             ss << "(...) -> ...;"; // Placeholder
    //         break;
         case SymbolInfo::STRUCT_KIND:
             ss << "struct " << symbol.name;
    //         // Struct yapısını serialize et (alanlar)
              StructType* structType = static_cast<StructType*>(symbol.type);
              ss << " {";
    //         // for (const auto& field_pair : structType->fields) { // Eğer alan bilgisi Type'ta tutuluyorsa
                  ss << field_pair.first << ": " << serializeType(field_pair.second) << ", ";
              }
              ss << "}";
             ss << " { ... }"; // Placeholder
             break;
         case SymbolInfo::VAR_KIND:
             ss << (symbol.isMutable ? "mut " : "let ") << symbol.name << ": " << serializeType(symbol.type) << ";";
             break;
    //     // ... Diğer sembol türleri (Enum, Trait vb.)
         default:
              ss << "// ERROR: Unknown symbol kind for '" << symbol.name << "'";
              break;
     }

    return ss.str(); // Oluşturulan stringi döndür
}

// Semantik tipi .hnt formatına serialize eder (string olarak)
// **ÖNEMLİ:** Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır!
std::string InterfaceExtractor::serializeType(const Type* type) const {
    if (!type) return "unknown";

    switch (type->id) {
        case Type::INT_TYPE: return "int";
        case Type::FLOAT_TYPE: return "float";
        case Type::BOOL_TYPE: return "bool";
        case Type::STRING_TYPE: return "string";
        case Type::CHAR_TYPE: return "char";
        case Type::VOID_TYPE: return "void";
        case Type::STRUCT_TYPE: return static_cast<const StructType*>(type)->declaration->name->name; // Struct adını yaz
        case Type::ENUM_TYPE: return static_cast<const EnumType*>(type)->declaration->name->name; // Enum adını yaz
        case Type::ARRAY_TYPE:
             // Array tipi: [ElementType] veya [ElementType; size]
              return "[" + serializeType(static_cast<const ArrayType*>(type)->elementType.get()) + "]";
             return "[...]"; // Placeholder
        case Type::REFERENCE_TYPE:
             // Referans tipi: &mut? ReferencedType
              const ReferenceType* refType = static_cast<const ReferenceType*>(type);
              return (refType->isMutable ? "&mut " : "&") + serializeType(refType->referencedType.get());
             return "&..."; // Placeholder
        case Type::FUNCTION_TYPE:
             // Fonksiyon tipi: fn(Args) -> Ret
              const FunctionType* funcType = static_cast<const FunctionType*>(type);
              std::stringstream ss;
              ss << "fn(";
              for (size_t i = 0; i < funcType->parameterTypes.size(); ++i) {
                  ss << serializeType(funcType->parameterTypes[i]);
                  if (i < funcType->parameterTypes.size() - 1) ss << ", ";
              }
              ss << ") -> " << serializeType(funcType->returnType);
              return ss.str();
             return "fn(...) -> ..."; // Placeholder
        // ... Diğer tipler (Pointer, Tuple vb.)
        default: return "unknown_type";
    }
}
