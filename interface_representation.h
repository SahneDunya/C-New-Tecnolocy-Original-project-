#ifndef CNT_COMPILER_INTERFACE_REPRESENTATION_H // Include guard güncellendi
#define CNT_COMPILER_INTERFACE_REPRESENTATION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem> // Dosya yolu için

// Semantic sistemden ihtiyaç duyulan başlıklar (ileri bildirimler veya tam tanımlar)
// SymbolInfo ve Type* yapıları daha önceki başlıklarda tanımlandı.
#include "symbol_table.h" // SymbolInfo için (SymbolInfo tanımı burada olmalı veya SymbolInfo da ayrı dosyada olmalı)
#include "type_system.h"  // Type* ve alt sınıfları için (Type temel sınıfı burada olmalı veya ayrı dosyada olmalı)

// Not: SymbolInfo ve Type yapılarının kendi başlık dosyalarında tanımlanması
// daha temiz bir yapı sağlayacaktır. Örneğin SymbolInfo için symbol_info.h,
// Type için type.h gibi. Şu anki yapıda symbol_table.h ve type_system.h'a
// dahil edilmiş olduklarını varsayıyoruz.


// Bir modülün public olarak dışa aktardığı arayüz bilgisi
struct ModuleInterface {
    std::string canonicalPath; // Modülün standart kanonik yolu (örn: "std::io")
    std::string filePath;      // Bu arayüzün kaynak dosyasının (.cnt) veya .hnt dosyasının yolu

    // Public olarak dışa aktarılan semboller
    // std::string -> sembol ismi
    // std::shared_ptr<SymbolInfo> -> sembole ait anlamsal bilgi (tipi, türü, mutability vb.)
    // SymbolInfo objeleri, arayüze eklenecek bilgiyi içermelidir (örn: fonksiyon imzası, struct alanları).
    // Bu SymbolInfo'lar, orijinal sembol tablosundaki SymbolInfo'ların kopyaları olabilir,
    // ancak orijinal AST düğümlerine (declarationNode) pointer tutmamalıdırlar,
    // çünkü AST'nin kendisi arayüz dosyasında saklanmaz.
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> publicSymbols;

    // (İsteğe bağlı) Public olarak dışa aktarılan tiplerin ayrı bir listesi
    // (Eğer SymbolInfo içinde tip bilgisi yeterli değilse)
     std::unordered_map<std::string, Type*> publicTypes;


    // Kurucu
    ModuleInterface(std::string path, std::string file_path)
        : canonicalPath(std::move(path)), filePath(std::move(file_path)) {}

    // Public sembol ekleme
    void addPublicSymbol(const std::string& name, std::shared_ptr<SymbolInfo> symbol) {
        if (!symbol) return; // Null pointer kontrolü
        // Eğer SymbolInfo shared_ptr içinde geliyorsa, onu kopyalamak yerine taşımak daha verimli olabilir.
        publicSymbols[name] = std::move(symbol);
    }

    // Public sembol arama (isme göre)
    std::shared_ptr<SymbolInfo> findPublicSymbol(const std::string& name) const {
        auto it = publicSymbols.find(name);
        if (it != publicSymbols.end()) {
            return it->second;
        }
        return nullptr; // Bulunamadı
    }

    // Arayüzdeki tüm public sembolleri döndür
    const std::unordered_map<std::string, std::shared_ptr<SymbolInfo>>& getPublicSymbols() const {
        return publicSymbols;
    }

};

#endif // CNT_COMPILER_INTERFACE_REPRESENTATION_H
