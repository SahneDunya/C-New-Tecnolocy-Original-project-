#include "module_manager.h"
#include <fstream>      // Dosya okuma/yazma için
#include <sstream>      // String stream için
#include <filesystem>   // Dosya sistemi işlemleri için (C++17)

// Semantik sistem implementasyonları (Include edilmez, başlık yeterli)
 #include "symbol_table.cpp"
 #include "type_system.cpp"


// Module Resolver Kurucu
ModuleResolver::ModuleResolver(Diagnostics& diag, TypeSystem& ts /*, SymbolTable& st*/)
    : diagnostics(diag), typeSystem(ts) /*, symbolTable(st)*/ {
    // Kurulum işlemleri
}

// Modül arama yollarını ayarlar
void ModuleResolver::setImportSearchPaths(const std::vector<std::string>& paths) {
    importSearchPaths.clear();
    for (const auto& p : paths) {
        importSearchPaths.push_back(std::filesystem::canonical(p)); // Yolları normalize et
    }
}

// Modül yolundan (örn: ["std", "io"]) beklenen .hnt dosyasının yolunu arama dizinlerinde bulur
std::optional<std::filesystem::path> ModuleResolver::findModuleInterfaceFile(const std::vector<std::string>& modulePathSegments) const {
    if (modulePathSegments.empty()) return std::nullopt;

    // Kanonik dosya adını oluştur (örn: "io.hnt")
    std::string filename = modulePathSegments.back() + ".hnt";

    // Modül yolunun kalanından dizin yolunu oluştur (örn: "std/")
    std::filesystem::path moduleDir;
    for (size_t i = 0; i < modulePathSegments.size() - 1; ++i) {
        moduleDir /= modulePathSegments[i];
    }

    // Her arama yolunda dosyayı ara
    for (const auto& searchPath : importSearchPaths) {
        std::filesystem::path fullPath = searchPath / moduleDir / filename;
        if (std::filesystem::exists(fullPath)) {
            return fullPath; // Bulunan ilk yolu döndür
        }
    }

    return std::nullopt; // Dosya bulunamadı
}

// Bir .hnt dosyasını okuyup ModuleInterface yapısına ayrıştırır (Deserialize)
// **ÖNEMLİ:** Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır!
std::shared_ptr<ModuleInterface> ModuleResolver::parseModuleInterfaceFile(const std::filesystem::path& filePath, const std::string& canonicalPath) {
    // .hnt dosyasını aç
    std::ifstream file(filePath);
    if (!file.is_open()) {
        diagnostics.reportInternalError("Modül arayüz dosyası açılamadı: " + filePath.string());
        return nullptr;
    }

    // ModuleInterface objesini oluştur
    auto moduleInterface = std::make_shared<ModuleInterface>(canonicalPath, filePath.string());

    // Dosya içeriğini oku ve arayüz bilgilerini (public semboller, tipler vb.) ayrıştır.
    // Bu, kendi yazacağınız bir .hnt parser'ı gerektirir.
    // Örneğin, dosya içinde "pub fn add(a: int, b: int) -> int;" gibi tanımlar olabilir.
    // Bu tanımları okuyacak, semantik tiplere (TypeSystem kullanarak) ve sembol bilgilerine (SymbolInfo) dönüştürecek
    // ve bunları moduleInterface->publicSymbols haritasına ekleyeceksiniz.

    // Örnek (Çok basit bir konsept, gerçek parser çok daha karmaşık olacaktır):
     std::string line;
     while (std::getline(file, line)) {
    //     // Satırı ayrıştır (örn: "pub fn name (args) -> ret_type ;")
    //     // İsimi, tipi vb. çıkar
    //     // TypeSystem'den semantik tipleri al
    //     // SymbolInfo oluştur
          moduleInterface->addPublicSymbol(name, symbolInfo);
     }

    // Eğer .hnt formatı başka modüllere bağımlılık (import) belirtiyorsa,
    // bu bağımlılıkları da burada çözümleyip yüklemelisiniz (recursive load).
     loadDependencies(*moduleInterface);

    file.close();
    return moduleInterface;
}

// İmport edilen sembolleri hedef sembol tablosuna ekler
void ModuleResolver::addImportedSymbolsToScope(const ModuleInterface& interface, SymbolTable& targetSymbolTable, const std::string& alias) {
    if (alias.empty()) {
        // Alias yoksa, public sembolleri doğrudan mevcut kapsama ekle
        for (const auto& pair : interface.publicSymbols) {
            const std::string& name = pair.first;
            std::shared_ptr<SymbolInfo> symbol = pair.second;

            // Sembolün mevcut kapsamda çakışmadığını kontrol et (isteğe bağlı, SEMA da yapabilir)
             if (targetSymbolTable.lookupCurrentScope(name)) {
                  diagnostics.reportError(... "İmport edilen isim '" + name + "' zaten mevcut kapsamda tanımlı.");
                  continue; // Bu sembolü ekleme
             }

            // Sembolü hedef sembol tablosuna ekle
            targetSymbolTable.insert(name, symbol);
             // Not: Sembol tablosu SymbolInfo'nun kopyasını mı tutacak, yoksa paylaşılan pointer mı?
             // shared_ptr kullanmak, farklı kapsamlardan aynı sembol bilgisine erişimi sağlar.
        }
    } else {
        // Alias varsa, modül arayüzünü alias ismiyle bir sembol olarak ekle
        // Bu sembolün tipi özel bir "Modül Tipi" olabilir veya sembol tablosunda bir scope'u işaret edebilir.
        // En basit yol, alias ismini bir "Namespace" veya "Module" sembolü olarak ekleyip,
        // bu sembolün altında import edilen public sembolleri saklamak.
        // Sembol tablonuzun namespace/scope sembollerini desteklemesi gerekir.

        // Örnek (Sembol tablosunun namespace/module sembollerini desteklediğini varsayarsak):
         auto moduleSymbol = std::make_shared<SymbolInfo>(alias, nullptr, nullptr, false); // Tipi özel ModuleType* olabilir
         moduleSymbol->kind = SymbolInfo::MODULE_KIND; // Sembol türünü belirt
         moduleSymbol->moduleInterface = &interface; // Veya ModuleInterface'e pointer/referans tut

         if (targetSymbolTable.insert(alias, moduleSymbol)) {
        //      // Başarılı ekleme, şimdi bu alias sembolü altında public sembolleri saklayın.
        //      // Sembol tablonuzun iç içe sembol haritalarını yönetmesi gerekir.
               targetSymbolTable.addSymbolsToNamespace(alias, interface.publicSymbols);
         } else {
             diagnostics.reportError(... "Alias ismi '" + alias + "' zaten mevcut kapsamda tanımlı.");
         }
        diagnostics.reportWarning(interface.filePath, 0, 0, "Aliasli import implementasyonu eksik."); // Placeholder
         // Alias implementasyonu daha karmaşık olacaktır.
    }
}

// Bir import ifadesini çözümleyip sembollerini hedef sembol tablosuna ekler
bool ModuleResolver::resolveImportStatement(ImportStatementAST* importStmt, SymbolTable& targetSymbolTable) {
    // Modül yolu segmentlerini al (örn: ["std", "io"])
    const auto& pathSegments = importStmt->path;
    if (pathSegments.empty()) {
        diagnostics.reportError(importStmt->location.filename, importStmt->location.line, importStmt->location.column,
                                "İmport ifadesinde modül yolu boş olamaz.");
        return false;
    }

    // Kanonik modül yolunu oluştur (örn: "std::io")
    std::string canonicalPath;
    for (size_t i = 0; i < pathSegments.size(); ++i) {
        canonicalPath += pathSegments[i];
        if (i < pathSegments.size() - 1) {
            canonicalPath += "::";
        }
    }

    // Cache'de ara
    std::shared_ptr<ModuleInterface> moduleInterface = getLoadedInterface(canonicalPath);

    if (!moduleInterface) {
        // Cache'de yoksa, .hnt dosyasını bulmaya çalış
        auto filePath = findModuleInterfaceFile(pathSegments);
        if (!filePath) {
            diagnostics.reportError(importStmt->location.filename, importStmt->location.line, importStmt->location.column,
                                    "Modül '" + canonicalPath + "' bulunamadı.");
            return false;
        }

        // .hnt dosyasını ayrıştır ve yükle
        moduleInterface = parseModuleInterfaceFile(*filePath, canonicalPath);
        if (!moduleInterface) {
            // Ayrıştırma hatası oluştu, hata zaten raporlandı
            return false;
        }

        // Yüklenen arayüzü cache'e ekle
        loadedInterfaces[canonicalPath] = moduleInterface;
    }

    // İmport edilen sembolleri hedef sembol tablosuna ekle
    // Alias varsa onu kullan, yoksa boş string geç
    std::string aliasName = importStmt->alias ? importStmt->alias->name : "";
    addImportedSymbolsToScope(*moduleInterface, targetSymbolTable, aliasName);

    diagnostics.reportInfo(importStmt->location.filename, importStmt->location.line, importStmt->location.column,
                           "Modül '" + canonicalPath + "' başarıyla yüklendi ve sembolleri eklendi."); // Bilgilendirme mesajı

    return true;
}

// Kanonik yoluna göre yüklenmiş bir modül arayüzünü döndürür (cache'den)
std::shared_ptr<ModuleInterface> ModuleResolver::getLoadedInterface(const std::string& canonicalPath) const {
    if (loadedInterfaces.count(canonicalPath)) {
        return loadedInterfaces.at(canonicalPath);
    }
    return nullptr;
}


// =======================================================================
// Otomatik Arayüz Çıkarımı (Çok karmaşık - İskelet Implementasyonu)
// Bu işlevler genellikle Semantic Analysis veya Code Generation sonrası çağrılır.
// =======================================================================

// Derlenmiş bir modülün (ProgramAST) public arayüzünü çıkarır
std::shared_ptr<ModuleInterface> ModuleResolver::extractInterface(ProgramAST* program) {
    // Program AST'sinin dosya adını al
    std::string filename = program->location.filename;
    // Dosya adından kanonik modül yolunu belirle (örn: src/my_module.cnt -> my_module veya std::my_module gibi)
    // Bu, projenin yapısına ve modül isimlendirme kurallarına bağlıdır.
    std::string canonicalPath = "unknown_module"; // Placeholder

    auto moduleInterface = std::make_shared<ModuleInterface>(canonicalPath, filename);

    // AST üzerindeki bildirimleri gez
    for (const auto& decl_ptr : program->declarations) {
        // Bildirimin public olup olmadığını belirle (CNT'de 'pub' anahtar kelimesi varsayalım)
        // AST düğümünüzde isPublic gibi bir bayrak olmalı veya attribute'ları kontrol etmelisiniz.
        bool isPublic = false; // Placeholder

        // Eğer public ise, bu bildirime ait semantik bilgiyi çıkar
        if (isPublic) {
            // Bildirimin türüne göre (Fonksiyon, Struct, Enum, Global Var)
            // İlgili semantik SymbolInfo veya Tip bilgisini al (SEMA tarafından AST'ye eklenmiş olmalı)
             SymbolInfo* symbol = decl_ptr->resolvedSymbol; // Eğer AST düğümünde sembol bilgisi varsa
             Type* semanticType = decl_ptr->resolvedSemanticType; // Veya tip bilgisi

            // Eğer geçerli sembol ve tip bilgisi varsa, ModuleInterface'e ekle
             if (symbol && semanticType) {
                  moduleInterface->addPublicSymbol(symbol->name, std::make_shared<SymbolInfo>(*symbol)); // SymbolInfo'nun kopyasını paylaş
                  Eğer SymbolInfo içinde pointerlar varsa (örn: DeclarationAST*), kopyalama dikkatli yapılmalı.
                 // Genellikle ModuleInterface, kendi SymbolInfo ve Type kopyalarını tutar.
             }
             diagnostics.reportWarning(decl_ptr->location.filename, decl_ptr->location.line, decl_ptr->location.column, "Arayüz çıkarımı implementasyonu eksik."); // Placeholder
        }
    }

    return moduleInterface;
}

// Çıkarılan modül arayüzünü .hnt dosyasına kaydeder (Serialize)
// **ÖNEMLİ:** Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır!
bool ModuleResolver::saveInterface(const ModuleInterface& interface, const std::filesystem::path& outputPath) {
    // Çıktı dizininin var olduğundan emin ol
    std::filesystem::create_directories(outputPath.parent_path());

    // .hnt dosyasını yazma modunda aç
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        diagnostics.reportError("", 0, 0, "Modül arayüz dosyası yazılamadı: " + outputPath.string());
        return false;
    }

    // ModuleInterface içeriğini (.hnt formatına göre) dosyaya yaz
    // Bu, publicSymbols'daki her sembolü veya publicDeclarations'daki her bildirimi
    // .hnt dosya formatının sözdizimine göre serialize edecek bir mantık gerektirir.
    // Örneğin, fonksiyon imzaları, struct alanları, enum varyantları gibi bilgileri yazmalısınız.

    // Örnek (Çok basit bir yazma konsepti):
     file << "// CNT Module Interface: " << interface.canonicalPath << std::endl;
     for (const auto& pair : interface.publicSymbols) {
         const std::string& name = pair.first;
         const SymbolInfo& symbol = *pair.second;
    //     // Sembol türüne göre formatlı yaz (örn: "pub fn name(args) -> ret_type;")
          file << "pub " << symbol.kind_string << " " << name << " : " << symbol.type->toString() << ";" << std::endl;
     }

    // Modülün bağımlılıklarını (importlarını) da .hnt dosyasına kaydetmek faydalı olabilir.

    file.close();
     diagnostics.reportInfo("", 0, 0, "Modül arayüz dosyası kaydedildi: " + outputPath.string()); // Bilgilendirme
    return true;
}

// Kanonik yoluna göre yüklenmiş bir modül arayüzünü döndürür (cache'den) - Tekrar tanım, cpp dosyasında olmalıydı
 std::shared_ptr<ModuleInterface> ModuleResolver::getLoadedInterface(const std::string& canonicalPath) const {
     if (loadedInterfaces.count(canonicalPath)) {
         return loadedInterfaces.at(canonicalPath);
     }
     return nullptr;
 }
