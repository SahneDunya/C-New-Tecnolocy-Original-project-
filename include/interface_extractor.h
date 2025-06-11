#ifndef CNT_COMPILER_INTERFACE_EXTRACTOR_H
#define CNT_COMPILER_INTERFACE_EXTRACTOR_H

#include "diagnostics.h"      // Hata raporlama
#include "ast.h"              // ProgramAST ve diğer AST düğümleri
#include "module_interface.h" // Çıkarılan arayüz bilgisini tutacak yapı
#include "symbol_table.h"     // Sembol bilgisi için (SEMA tarafından eklenmiş SymbolInfo)
#include "type_system.h"      // Tip bilgisi için (SEMA tarafından eklenmiş Type*)

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
     SymbolTable& symbolTable; // Belki global sembollere erişim için gerekli olabilir.

    // =======================================================================
    // Arayüz Çıkarımı Yardımcı Metodlar
    // =======================================================================

    // AST üzerinde gezerek public bildirimleri bulur ve arayüze ekler
    void extractPublicItems(const ASTNode* node, ModuleInterface& interface);

    // Bir bildirimin public olup olmadığını belirler (AST'deki isPublic bayrağına bakar)
    bool isPublic(const ASTNode* node) const;

    // Public bir bildirimin semantik bilgilerini (SymbolInfo, Type) kullanarak
    // ModuleInterface'e eklenecek SymbolInfo objesini oluşturur.
    // Bu SymbolInfo, orijinal AST düğümüne pointer tutmamalıdır, sadece arayüz bilgisini içermelidir.
    std::shared_ptr<SymbolInfo> buildInterfaceSymbolInfo(const DeclarationAST* publicDecl);

    // =======================================================================
    // Arayüz Kaydetme (Serialization) Yardımcı Metodlar
    // Bu metodlar ModuleInterface'deki bilgiyi .hnt formatına dönüştürür.
    // **ÖNEMLİ:** Bu implementasyon, .hnt dosya formatınıza bağlıdır!
    // =======================================================================

    // Sembol bilgilerini .hnt formatına serialize eder (string olarak)
    // Sembol türüne (fonksiyon, struct, var vb.) ve tipine göre formatlama yapar.
    std::string serializeSymbolInfo(const SymbolInfo& symbol) const;

    // Semantik tipi .hnt formatına serialize eder (string olarak)
    // Tipin yapısına (temel, referans, dizi, fonksiyon, struct vb.) göre formatlama yapar.
    std::string serializeType(const Type* type) const;

    // =======================================================================


public:
    // Kurucu
    // TypeSystem'e ihtiyaç var (deserialize etme için), SymbolTable'a da gerekebilir (global semboller vb.)
    InterfaceExtractor(Diagnostics& diag, TypeSystem& ts); // SymbolTable'ı da alabilir


    // Semantik analizi yapılmış bir ProgramAST'ten public arayüzü çıkarır
    // program: Derlenen modülün kök AST'si (SEMA sonrası, resolved bilgilerle)
    // moduleName: Modülün kanonik adı (örn: "std::io")
    // sourceFilePath: Modülün orijinal kaynak dosya yolu
    // Returns: Public arayüzü içeren ModuleInterface objesi. Hata durumunda nullptr.
    std::shared_ptr<ModuleInterface> extract(ProgramAST* program, const std::string& moduleName, const std::filesystem::path& sourceFilePath);

    // Çıkarılan ModuleInterface objesini .hnt dosyasına kaydeder (Serialize)
    // interface: Kaydedilecek arayüz objesi
    // outputPath: .hnt dosyasının yazılacağı dosya sistemi yolu
    // Returns: Başarı durumunda true, hata durumunda false.
    bool save(const ModuleInterface& interface, const std::filesystem::path& outputPath);

    // Define the .hnt file extension
    static constexpr const char* interfaceFileExtension = ".hnt";
};

#endif // CNT_COMPILER_INTERFACE_EXTRACTOR_H
