#include "module_manager.h"
#include "utils.h" // cnt_compiler::splitString gibi yardımcılar için

#include <fstream>      // Dosya okuma için
#include <sstream>      // String stream için
#include <filesystem>   // Dosya sistemi işlemleri için (C++17)

// Module Resolver Kurucu
ModuleResolver::ModuleResolver(Diagnostics& diag, TypeSystem& ts)
    : diagnostics(diag), typeSystem(ts) {
    // Kurulum işlemleri
}

// Modül arama yollarını ayarlar
void ModuleResolver::setImportSearchPaths(const std::vector<std::string>& paths) {
    importSearchPaths.clear();
    for (const auto& p : paths) {
         addImportSearchPath(p); // Her yolu addImportSearchPath ile ekle (normalize etmek için)
    }
}

// Bir arama yolu ekler (normalize edilmiş)
void ModuleResolver::addImportSearchPath(const std::filesystem::path& path) {
     std::error_code ec;
     std::filesystem::path canonicalPath = std::filesystem::canonical(path, ec);
     if (ec) {
          diagnostics.reportWarning("", 0, 0, "Import arama yolu '" + path.string() + "' geçerli değil: " + ec.message());
          return; // Geçersiz yolu ekleme
     }
     importSearchPaths.push_back(canonicalPath);
}


// Modül yolundan (örn: ["std", "io"]) beklenen .hnt dosyasının yolunu arama dizinlerinde bulur
std::optional<std::filesystem::path> ModuleResolver::findModuleInterfaceFile(const std::vector<std::string>& modulePathSegments) const {
    if (modulePathSegments.empty()) return std::nullopt;

    // Kanonik dosya adını oluştur (örn: "io.hnt")
    std::string filename = modulePathSegments.back() + InterfaceExtractor::interfaceFileExtension;

    // Modül yolunun kalanından dizin yolunu oluştur (örn: "std/")
    std::filesystem::path moduleDir;
    for (size_t i = 0; i < modulePathSegments.size() - 1; ++i) {
        moduleDir /= modulePathSegments[i];
    }

    // Her arama yolunda dosyayı ara
    for (const auto& searchPath : importSearchPaths) {
        std::filesystem::path fullPath = searchPath / moduleDir / filename;
        std::error_code ec; // Hata kodu için
        if (std::filesystem::exists(fullPath, ec)) {
            if (!ec) return fullPath; // Bulunan ilk yolu döndür (hata yoksa)
            else diagnostics.reportWarning("", 0, 0, "Dosya varlık kontrol hatası: " + fullPath.string() + " - " + ec.message());
        }
    }

    return std::nullopt; // Dosya bulunamadı
}

// Bir .hnt dosyasını okuyup ModuleInterface yapısına ayrıştırır (Deserialize)
// **ÖNEMLİ:** Bu metodun implementasyonu, .hnt dosya formatınıza bağlıdır!
// .hnt'deki CNT arayüz bilgilerini SymbolInfo ve Type* objelerine dönüştürür.
// C/C++ deklarasyonlarını ayrıştırmayı da içerebilir veya bunları farklı ele alabilir.
std::shared_ptr<ModuleInterface> ModuleResolver::parseModuleInterfaceFile(const std::filesystem::path& filePath, const std::string& canonicalPath) {
    // .hnt dosyasını aç
    std::ifstream file(filePath);
    if (!file.is_open()) {
        diagnostics.reportError("", 0, 0, "Modül arayüz dosyası açılamadı: " + filePath.string());
        return nullptr;
    }

    // ModuleInterface objesini oluştur
    auto moduleInterface = std::make_shared<ModuleInterface>(canonicalPath, filePath.string());

    // Dosya içeriğini satır satır veya bloğa göre oku ve arayüz bilgilerini ayrıştır.
    // Bu kısım, .hnt dosya formatınızın sözdizimini/yapısını anlayan bir parser gerektirir.
    // Sembol isimlerini, türlerini (serialize edilmiş string formatında) ve diğer public özellikleri okuyacaksınız.
    // Okuduğunuz tip stringlerini (örn: "fn(int, string) -> bool"), Tip Sistemini (typeSystem) kullanarak
    // karşılık gelen semantik Type* objelerine dönüştürmeniz gerekecek (deserialize).

    std::string line;
    int lineNumber = 0; // Hata raporlama için satır numarası
    while (std::getline(file, line)) {
        lineNumber++;
        line = cnt_compiler::trim(line); // Baştaki/sondaki boşlukları kırp
        if (line.empty() || line.rfind("//", 0) == 0) continue; // Boş satırları ve yorumları atla

        // Örnek Ayrıştırma Mantığı İskeleti (Basit "pub fn name(...) -> type;" formatını varsayar)
        // Gerçek bir parser çok daha karmaşık olacaktır.

        if (line.rfind("pub ", 0) == 0) { // "pub " ile başlıyorsa
            std::string publicDecl = cnt_compiler::trim(line.substr(4)); // "pub " kısmını atla

            // Bildirim türünü belirle (fn, struct, enum, let/mut)
            if (publicDecl.rfind("fn ", 0) == 0) {
                // Fonksiyon bildirimi ayrıştır (isim, parametre tipleri, dönüş tipi)
                 stringstream ss(publicDecl.substr(3)); // "fn " kısmını atla
                 std::string funcName; ss >> funcName;
                // ... Parametre ve dönüş tipi stringlerini oku
                 Type* semanticFuncType = typeSystem.deserializeFunctionType(ss); // Tip Sisteminde böyle bir metod olmalı
                 if (semanticFuncType) {
                      auto funcSymbol = std::make_shared<SymbolInfo>(funcName, semanticFuncType, nullptr, false);
                      funcSymbol->kind = SymbolInfo::FUNCTION_KIND; // Sembol türünü ayarla
                      moduleInterface->addPublicSymbol(funcName, funcSymbol);
                 } else {
                      diagnostics.reportError(filePath.string(), lineNumber, 0, "Fonksiyon imzası ayrıştırılamadı: " + line);
                 }
                 diagnostics.reportWarning(filePath.string(), lineNumber, 0, ".hnt ayrıştırıcı implementasyonu eksik (Fonksiyon). Satır: " + line); // Placeholder

            } else if (publicDecl.rfind("struct ", 0) == 0) {
                 // Struct bildirimi ayrıştır (isim, alanlar ve tipleri)
                  stringstream ss(publicDecl.substr(7)); // "struct " kısmını atla
                  std::string structName; ss >> structName;
                 // ... Alan isimlerini ve tip stringlerini oku
                  Type* semanticStructType = typeSystem.deserializeStructType(ss); // Tip Sisteminde böyle bir metod olmalı
                  if (semanticStructType) {
                      auto structSymbol = std::make_shared<SymbolInfo>(structName, semanticStructType, nullptr, false);
                      structSymbol->kind = SymbolInfo::STRUCT_KIND; // Sembol türünü ayarla
                      moduleInterface->addPublicSymbol(structName, structSymbol);
                  } else {
                      diagnostics.reportError(filePath.string(), lineNumber, 0, "Struct tanımı ayrıştırılamadı: " + line);
                  }
                  diagnostics.reportWarning(filePath.string(), lineNumber, 0, ".hnt ayrıştırıcı implementasyonu eksik (Struct). Satır: " + line); // Placeholder

            } // ... Diğer bildirim türleri (enum, global var)

            else {
                 diagnostics.reportWarning(filePath.string(), lineNumber, 0, "Tanımlanamayan public bildirim formatı: " + line);
            }
        }
        // C/C++ deklarasyonları da .hnt içindeyse, onları burada parse edip ayrı tutmanız veya
        // farklı bir şekilde işlemeniz gerekebilir.

    }

    file.close();

    // (Opsiyonel) loadDependencies çağrısı yapılabilir eğer .hnt dosyası başka importlar belirtiyorsa.

    return moduleInterface;
}


// İmport edilen sembolleri hedef sembol tablosuna ekler
void ModuleResolver::addImportedSymbolsToScope(const ModuleInterface& interface, SymbolTable& targetSymbolTable, const std::string& alias) {
    if (alias.empty()) {
        // Alias yoksa, public sembolleri doğrudan mevcut kapsama ekle
        // Sembol tablosunun insert metodu çakışmaları yönetmelidir.
        for (const auto& pair : interface.publicSymbols) {
            const std::string& name = pair.first;
            std::shared_ptr<SymbolInfo> symbol = pair.second;

            // Sembolü hedef sembol tablosuna ekle.
            // Eğer çakışma olursa insert false döner veya hata raporlanır (Sembol Tablosu implementasyonu).
            // Çakışma durumunda SEMA'nın hata raporlaması daha uygun olabilir,
            // ancak ModuleResolver burada da bir uyarı verebilir.
             if (!targetSymbolTable.insert(name, symbol)) {
                 // Sembol Tablosu insert hata raporlamıyorsa, burada biz yapmalıyız.
                 // SEMA'nın import deyimini analiz ederken sembol çözümlemesi yapması ve
                 // isim çakışmasını SEMA'da yakalaması daha yaygın bir yaklaşımdır.
                 // Bu durumda bu addImportedSymbolsToScope sadece sembolleri ekler, çakışma kontrolünü SEMA'ya bırakır.
                  diagnostics.reportWarning(... "Import edilen isim '" + name + "' mevcut kapsamda çakışıyor.");
             }
        }
    } else {
        // Alias varsa, modül arayüzünü alias ismiyle bir namespace/modül sembolü olarak ekle
        // Bu, sembol tablosunun namespace/scope sembollerini desteklemesini gerektirir.
        // import io as sys; -> sys.print() şeklinde kullanım için.

        // Örnek: Alias ismini bir sembol olarak ekle (Sembol tablosu namespace/modül türünü desteklemeli)
         auto moduleSymbol = std::make_shared<SymbolInfo>(alias, nullptr, nullptr, false); // Type özel ModuleType* olabilir
         moduleSymbol->kind = SymbolInfo::MODULE_KIND; // Sembol türünü belirt
         moduleSymbol->moduleInterface = &interface; // Veya ModuleInterface'e pointer/referans tutulabilir

         if (targetSymbolTable.insert(alias, moduleSymbol)) {
        //      // Başarılı ekleme, şimdi bu alias sembolü altında public sembolleri erişilebilir yapın.
        //      // Bu SymbolTable implementasyonuna bağlıdır. Belki alias sembolü altındaki scope'a sembolleri ekleriz.
               targetSymbolTable.addSymbolsToNamespace(alias, interface.publicSymbols);
              diagnostics.reportWarning(interface.filePath, 0, 0, "Aliasli importların sembol tablosuna eklenmesi implemente edilmedi."); // Placeholder
         } else {
             diagnostics.reportError(... "Alias ismi '" + alias + "' mevcut kapsamda çakışıyor.");
         }
        diagnostics.reportWarning(interface.filePath, 0, 0, "Aliasli import implementasyonu eksik."); // Placeholder
    }
}


// Bir import ifadesini çözümleyip sembollerini hedef sembol tablosuna ekler
bool ModuleResolver::resolveImportStatement(ImportStatementAST* importStmt, SymbolTable& targetSymbolTable) {
    if (!importStmt) return false;

    // Modül yolu segmentlerini al (örn: ["std", "io"])
    const auto& pathSegments = importStmt->path;
    if (pathSegments.empty()) {
        // Bu hata parser'da yakalanmalı, ama burada da kontrol edebiliriz.
        diagnostics.reportError(importStmt->location, "İmport ifadesinde modül yolu boş olamaz.");
        return false;
    }

    // Kanonik modül yolunu oluştur (örn: "std::io")
    std::string canonicalPath = cnt_compiler::joinStrings(pathSegments, "::");

    // Cache'de ara
    std::shared_ptr<ModuleInterface> moduleInterface = getLoadedInterface(canonicalPath);

    if (!moduleInterface) {
        // Cache'de yoksa, .hnt dosyasını bulmaya çalış
        auto filePath = findModuleInterfaceFile(pathSegments);
        if (!filePath) {
            diagnostics.reportError(importStmt->location, "Modül '" + canonicalPath + "' bulunamadı. Arama yolları kontrol edildi.");
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
        diagnostics.reportInfo(importStmt->location, "Modül arayüzü başarıyla yüklendi: '" + canonicalPath + "' from '" + filePath->string() + "'");
    } else {
        // Cache'den yüklendi
         diagnostics.reportInfo(importStmt->location, "Modül arayüzü cache'den yüklendi: '" + canonicalPath + "'");
    }

    // İmport edilen sembolleri hedef sembol tablosuna ekle
    // Alias varsa onu kullan, yoksa boş string geç
    std::string aliasName = importStmt->alias ? importStmt->alias->name : "";
    addImportedSymbolsToScope(*moduleInterface, targetSymbolTable, aliasName);

    // Çözümlenen arayüzü AST düğümüne ekle (SEMA bu pointer'ı kullanır)
    importStmt->resolvedInterface = moduleInterface;

    return true;
}

// Kanonik yoluna göre yüklenmiş bir modül arayüzünü döndürür (cache'den veya yükler)
std::shared_ptr<ModuleInterface> ModuleResolver::getLoadedInterface(const std::string& canonicalPath) {
    // Burada sadece cache'den dönme mantığı var.
    // Eğer istenirse, cache'de yoksa burada find/parse/cache yapma mantığı da eklenebilir,
    // ancak resolveImportStatement'ın ana giriş noktası olması daha yaygın.
    // Bu metod, daha çok dahili kullanım veya hata ayıklama için cache'e erişim sağlar.

    if (loadedInterfaces.count(canonicalPath)) {
        return loadedInterfaces.at(canonicalPath);
    }
    // Cache'de yoksa null döner. resolveImportStatement bulma ve yükleme işlemini yapar.
    return nullptr;
}
