#ifndef CNT_COMPILER_MODULE_MANAGER_H
#define CNT_COMPILER_MODULE_MANAGER_H

#include "diagnostics.h"        // Hata raporlama
#include "ast.h"                // ImportStatementAST için
#include "symbol_table.h"       // Sembol tablosu ile etkileşim için
#include "type_system.h"        // Tip sistemi ile etkileşim için
#include "module_interface.h"   // Modül arayüz yapısı için

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>           // Dosya sistemi işlemleri için (C++17)
#include <optional>             // std::optional için (C++17)

// Modül Çözümleyici Sınıfı
class ModuleResolver {
private:
    Diagnostics& diagnostics; // Hata raporlama sistemi
    TypeSystem& typeSystem;   // Derleyicinin ana Tip Sistemi referansı
    // SymbolTable& symbolTable; // Derleyicinin ana Sembol Tablosu referansı (import edildiği yere bağlı olarak farklı olabilir)

    // Yüklenmiş modül arayüzlerinin cache'i
    // Kanonik modül yolu (örn: "std::io") -> ModuleInterface pointer'ı
    std::unordered_map<std::string, std::shared_ptr<ModuleInterface>> loadedInterfaces;

    // .hnt dosyalarını aramak için dizin listesi
    std::vector<std::filesystem::path> importSearchPaths;

    // =======================================================================
    // Yardımcı Metodlar
    // =======================================================================

    // Modül yolundan (örn: ["std", "io"]) beklenen .hnt dosyasının yolunu arama dizinlerinde bulur
    std::optional<std::filesystem::path> findModuleInterfaceFile(const std::vector<std::string>& modulePathSegments) const;

    // Bir .hnt dosyasını okuyup ModuleInterface yapısına ayrıştırır (Deserialize)
    // Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır.
    std::shared_ptr<ModuleInterface> parseModuleInterfaceFile(const std::filesystem::path& filePath, const std::string& canonicalPath);

    // İmport edilen sembolleri hedef sembol tablosuna ekler
    void addImportedSymbolsToScope(const ModuleInterface& interface, SymbolTable& targetSymbolTable, const std::string& alias = "");

    // Recursive bağımlılık yükleme (Bir modülün arayüzü başka modülleri import ediyorsa)
     void loadDependencies(ModuleInterface& interface);


public:
    // Kurucu
    // Tip Sistemi ve Sembol Tablosu'na ihtiyaç duyabilir (import edilenleri entegre etmek için)
    ModuleResolver(Diagnostics& diag, TypeSystem& ts /*, SymbolTable& st*/);

    // Modül arama yollarını ayarlar
    void setImportSearchPaths(const std::vector<std::string>& paths);

    // Bir import ifadesini çözümleyip sembollerini hedef sembol tablosuna ekler
    // Semantic Analyzer tarafından çağrılır.
    // Başarılı olursa true döner.
    bool resolveImportStatement(ImportStatementAST* importStmt, SymbolTable& targetSymbolTable);

    // Kanonik yoluna göre yüklenmiş bir modül arayüzünü döndürür (cache'den)
    std::shared_ptr<ModuleInterface> getLoadedInterface(const std::string& canonicalPath) const;


    // =======================================================================
    // Otomatik Arayüz Çıkarımı (Çok karmaşık - İskelet)
    // Bu genellikle Semantic Analysis veya Code Generation sonrası yapılır.
    // =======================================================================

    // Derlenmiş bir modülün (ProgramAST) public arayüzünü çıkarır
    // Hangi bildirimlerin public olduğunu belirlemek için AST üzerinde gezinir.
    std::shared_ptr<ModuleInterface> extractInterface(ProgramAST* program);

    // Çıkarılan modül arayüzünü .hnt dosyasına kaydeder (Serialize)
    // Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır.
    bool saveInterface(const ModuleInterface& interface, const std::filesystem::path& outputPath);
};

#endif // CNT_COMPILER_MODULE_MANAGER_H
