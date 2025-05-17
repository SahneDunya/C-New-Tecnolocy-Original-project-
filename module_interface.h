#ifndef CNT_COMPILER_MODULE_INTERFACE_H
#define CNT_COMPILER_MODULE_INTERFACE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Semantic sistemden ihtiyaç duyulan başlıklar (ileri bildirimler veya tam tanımlar)
#include "symbol_table.h" // SymbolInfo için
#include "type_system.h"  // Type* ve alt sınıfları için

// Bir modülün public olarak dışa aktardığı arayüz bilgisi
struct ModuleInterface {
    std::string canonicalPath; // Modülün standart yolu (örn: "std::io")
    std::string filePath;      // Arayüz dosyasının (.hnt) dosya sistemindeki yolu

    // Public olarak dışa aktarılan semboller
     std::string -> sembol ismi
     std::shared_ptr<SymbolInfo> -> sembole ait anlamsal bilgi (tipi, türü, mutability vb.)
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> publicSymbols;

    // (İsteğe bağlı) Public olarak dışa aktarılan tiplerin ayrı bir listesi
     std::unordered_map<std::string, Type*> publicTypes;

    // Kurucu
    ModuleInterface(std::string path, std::string file_path)
        : canonicalPath(std::move(path)), filePath(std::move(file_path)) {}

    // Public sembol ekleme
    void addPublicSymbol(const std::string& name, std::shared_ptr<SymbolInfo> symbol) {
        publicSymbols[name] = std::move(symbol);
    }

    // Public sembol arama
    std::shared_ptr<SymbolInfo> findPublicSymbol(const std::string& name) const {
        if (publicSymbols.count(name)) {
            return publicSymbols.at(name);
        }
        return nullptr;
    }
};

#endif // CNT_COMPILER_MODULE_INTERFACE_H
