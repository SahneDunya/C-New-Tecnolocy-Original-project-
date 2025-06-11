#ifndef CNT_COMPILER_MODULE_MANAGER_H
#define CNT_COMPILER_MODULE_MANAGER_H

#include "diagnostics.h"        // Hata raporlama
#include "ast.h"                // ImportStatementAST için
#include "symbol_table.h"       // Sembol tablosu ile etkileşim için (SymbolTable, SymbolInfo)
#include "type_system.h"        // Tip sistemi ile etkileşim için (TypeSystem, Type)
#include "module_interface.h"   // Modül arayüz yapısı için (ModuleInterface)

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>           // Dosya sistemi işlemleri için (C++17)
#include <optional>             // std::optional için (C++17)

// Module Resolver Sınıfı
class ModuleResolver {
private:
    Diagnostics& diagnostics; // Hata raporlama sistemi
    TypeSystem& typeSystem;   // Derleyicinin ana Tip Sistemi referansı (import edilen tipleri kaydetmek için)
    // SymbolTable& symbolTable; // SEMA'dan hedef SymbolTable'ı alacak, burada referans tutmaya gerek yok.

    // Yüklenmiş modül arayüzlerinin cache'i
    // Kanonik modül yolu (örn: "std::io") -> ModuleInterface pointer'ı
    std::unordered_map<std::string, std::shared_ptr<ModuleInterface>> loadedInterfaces;

    // .hnt dosyalarını aramak için dizin listesi
    std::vector<std::filesystem::path> importSearchPaths;

    // =======================================================================
    // Yardımcı Metodlar (Dosya Bulma ve Ayrıştırma)
    // =======================================================================

    // Modül yolundan (örn: ["std", "io"]) beklenen .hnt dosyasının yolunu arama dizinlerinde bulur
    std::optional<std::filesystem::path> findModuleInterfaceFile(const std::vector<std::string>& modulePathSegments) const;

    // Bir .hnt dosyasını okuyup ModuleInterface yapısına ayrıştırır (Deserialize)
    // Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır.
    // .hnt'deki CNT arayüz bilgilerini SymbolInfo ve Type* objelerine dönüştürür.
    // C/C++ deklarasyonlarını ayrıştırmayı da içerebilir veya bunları farklı ele alabilir.
    std::shared_ptr<ModuleInterface> parseModuleInterfaceFile(const std::filesystem::path& filePath, const std::string& canonicalPath);

    // İmport edilen sembolleri hedef sembol tablosuna ekler
    // Alias mantığını ve isim çakışmalarını yönetir.
    void addImportedSymbolsToScope(const ModuleInterface& interface, SymbolTable& targetSymbolTable, const std::string& alias = "");

    // Recursive bağımlılık yükleme (Bir modülün arayüzü başka modülleri import ediyorsa)
    // parseModuleInterfaceFile içinde çağrılabilir.
     void loadDependencies(ModuleInterface& interface);


public:
    // Kurucu
    // Tip Sistemine ihtiyacı var (import edilen tipleri kaydetmek/çözmek için).
    // Diagnostics zaten var.
    ModuleResolver(Diagnostics& diag, TypeSystem& ts);

    // Modül arama yollarını ayarlar
    void setImportSearchPaths(const std::vector<std::string>& paths);
    // Bir arama yolu ekler
    void addImportSearchPath(const std::filesystem::path& path);


    // Bir import ifadesini çözümleyip sembollerini hedef sembol tablosuna ekler
    // Semantic Analyzer tarafından çağrılır.
    // Başarılı olursa true döner. Başarısız olursa hata raporlar.
    bool resolveImportStatement(ImportStatementAST* importStmt, SymbolTable& targetSymbolTable);

    // Kanonik yoluna göre yüklenmiş bir modül arayüzünü döndürür (cache'den veya yükler)
    // Bu metod, resolveImportStatement tarafından dahili olarak kullanılır.
    // Başarılı olursa ModuleInterface pointer'ı, başarısız olursa nullptr döner.
    std::shared_ptr<ModuleInterface> getLoadedInterface(const std::string& canonicalPath);
};

#endif // CNT_COMPILER_MODULE_MANAGER_H
