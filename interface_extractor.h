#ifndef CNT_COMPILER_INTERFACE_EXTRACTOR_H
#define CNT_COMPILER_INTERFACE_EXTRACTOR_H

#include "diagnostics.h"      // Hata raporlama
#include "ast.h"              // ProgramAST ve diğer AST düğümleri
#include "module_interface.h" // Çıkarılan arayüz bilgisini tutacak yapı
#include "symbol_table.h"     // Sembol bilgisi için (SEMA tarafından eklenmiş)
#include "type_system.h"      // Tip bilgisi için (SEMA tarafından eklenmiş)

#include <string>
#include <vector>
#include <memory>
#include <filesystem>         // Dosya kaydetme işlemleri için (C++17)
#include <unordered_map>

// Interface Extractor Sınıfı
class InterfaceExtractor {
private:
    Diagnostics& diagnostics; // Hata raporlama sistemi
    TypeSystem& typeSystem;   // Tip sistemine erişim (tipleri serialize etmek için)

    // =======================================================================
    // Arayüz Çıkarımı Yardımcı Metodlar
    // =======================================================================

    // AST üzerinde gezerek public bildirimleri bulur ve arayüze ekler
    void extractPublicItems(const ASTNode* node, ModuleInterface& interface);

    // Bir bildirimin public olup olmadığını belirler (CNT dilinin kuralına göre - 'pub' anahtar kelimesi vb.)
    bool isPublic(const ASTNode* node) const;

    // Public bir bildirimin semantik bilgilerini (SymbolInfo, Type) çıkarır
    // Bu bilgiler SEMA tarafından AST düğümlerine eklenmiş olmalıdır.
    // ModuleInterface'e eklenecek SymbolInfo objesini oluşturur (pointerları temizlenmiş, sadece public data)
    std::shared_ptr<SymbolInfo> buildInterfaceSymbolInfo(const ASTNode* publicNode);

    // =======================================================================
    // Arayüz Kaydetme (Serialization) Yardımcı Metodlar
    // Bu metodlar ModuleInterface'deki bilgiyi .hnt formatına dönüştürür.
    // =======================================================================

    // Sembol bilgilerini .hnt formatına serialize eder (string olarak)
    std::string serializeSymbolInfo(const SymbolInfo& symbol) const;

    // Semantik tipi .hnt formatına serialize eder (string olarak)
    std::string serializeType(const Type* type) const;

    // =======================================================================


public:
    // Kurucu
    InterfaceExtractor(Diagnostics& diag, TypeSystem& ts);

    // Semantik analizi yapılmış bir ProgramAST'ten public arayüzü çıkarır
    // program: Derlenen modülün kök AST'si
    // moduleName: Modülün kanonik adı (örn: "std::io")
    // sourceFilePath: Modülün orijinal kaynak dosya yolu
    // Returns: Public arayüzü içeren ModuleInterface objesi. Hata durumunda nullptr.
    std::shared_ptr<ModuleInterface> extract(ProgramAST* program, const std::string& moduleName, const std::filesystem::path& sourceFilePath);

    // Çıkarılan ModuleInterface objesini .hnt dosyasına kaydeder
    // interface: Kaydedilecek arayüz objesi
    // outputPath: .hnt dosyasının yazılacağı dosya sistemi yolu
    // Returns: Başarı durumunda true, hata durumunda false.
    bool save(const ModuleInterface& interface, const std::filesystem::path& outputPath);

    // .hnt dosya uzantısı
    static constexpr const char* interfaceFileExtension = ".hnt";
};

#endif // CNT_COMPILER_INTERFACE_EXTRACTOR_H
