#ifndef CNT_COMPILER_MODULE_RESOLVER_H
#define CNT_COMPILER_MODULE_RESOLVER_H

#include "ast.h" // ImportStatementAST için
#include "diagnostics.h" // Hata raporlama için
#include "symbol_table.h" // İmport edilen sembolleri eklemek için
#include "type_system.h" // İmport edilen tipleri kaydetmek için

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// İleri bildirimler
 struct AST; // Belki import edilen modülün AST'sinin bir kısmına ihtiyaç duyulabilir?
// Veya sadece arayüz temsili yeterlidir.

// İmport edilen bir modülün arayüzünü temsil eden yapı
struct ModuleInterface {
    std::string moduleName;
    // İmport edilebilecek public semboller
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> publicSymbols; // İsimden sembol bilgisine (tip, tür vb.)

    // Belki public struct/enum/trait tanımlarının AST düğümlerinin kopyaları veya referansları?
     std::vector<std::shared_ptr<DeclarationAST>> publicDeclarations;
};

// Modül Çözümleyici Sınıfı
class ModuleResolver {
private:
    Diagnostics& diagnostics; // Hata raporlama
    // Belki Tip Sistemi ve Sembol Tablosu'na da ihtiyaç duyar? (İmport edilen tipleri/sembolleri kaydetmek için)
     TypeSystem& typeSystem;
    // SymbolTable& symbolTable; // Nereye import edildiğini bilmesi gerekir

    // Daha önce çözümlenmiş modüllerin cache'i
    std::unordered_map<std::string, std::shared_ptr<ModuleInterface>> resolvedModules;

    // İmport yollarını arayacağı dizinler listesi (Makefile veya komut satırından gelir)
    std::vector<std::string> importSearchPaths;

    // .hnt dosyasını okuyup ModuleInterface yapısına dönüştüren metod (Veya özel formatı okuyan)
    std::shared_ptr<ModuleInterface> loadModuleInterface(const std::string& modulePath, const TokenLocation& importLoc);

    // Otomatik arayüz çıkarımı: Derleme sonrası bir modülün arayüzünü çıkarıp kaydetme/dosyaya yazma
     void extractAndSaveInterface(ProgramAST* program, const std::string& outputPath);


public:
    ModuleResolver(Diagnostics& diag/*, TypeSystem& ts, SymbolTable& st*/);

    // Import arama yollarını ayarla
    void setImportSearchPaths(const std::vector<std::string>& paths);

    // Bir import ifadesini çözümle
    // Başarılı olursa true döner, çözümlenen sembolleri targetScope'a ekler.
    bool resolveImport(ImportStatementAST* importStmt, std::shared_ptr<Scope> targetScope); // Nereye import edileceğini belirt

    // Başka modüller tarafından kullanılmak üzere mevcut modülün arayüzünü çıkar
    // Bu genellikle semantic analysis veya code generation sonrası yapılır.
     void exportModuleInterface(ProgramAST* program, const std::string& moduleName);
};

#endif // CNT_COMPILER_MODULE_RESOLVER_H
