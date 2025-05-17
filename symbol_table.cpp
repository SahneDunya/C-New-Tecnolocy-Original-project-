#include "symbol_table.h"
#include "utils.h" // cnt_compiler::getSymbolKindString için

#include <iostream>
#include <iomanip> // setw için
#include <cassert> // assert için

namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // SymbolKind enum değerini string olarak döndüren yardımcı fonksiyon
    std::string getSymbolKindString(SymbolKind kind) {
        switch (kind) {
            case SymbolKind::Unknown: return "Unknown";
            case SymbolKind::Variable: return "Variable";
            case SymbolKind::Function: return "Function";
            case SymbolKind::Struct: return "Struct";
            case SymbolKind::Enum: return "Enum";
            case SymbolKind::EnumVariant: return "EnumVariant";
            case SymbolKind::StructField: return "StructField";
            case SymbolKind::Module: return "Module";
            // ... diğer türler
            default: return "UnknownKind"; // Düzgün handle edilmeyen bir enum değeri
        }
    }


    // SymbolTable Kurucu
    SymbolTable::SymbolTable() : currentScope(nullptr) {
        // Global kapsamı ilk kez enterScope çağrıldığında oluşturacağız.
    }

    // Yeni bir kapsam açar
    void SymbolTable::enterScope() {
        int newDepth = (currentScope == nullptr) ? 0 : currentScope->depth + 1;
        // Yeni kapsam objesini oluştur ve unique_ptr ile sahipliğini al.
        auto newScope = std::make_unique<Scope>(currentScope, newDepth);
        // Yeni kapsamı vektöre ekle ve currentScope'u güncelle.
        currentScope = newScope.get(); // unique_ptr'ın yönettiği objenin ham pointer'ı
        scopes.push_back(std::move(newScope)); // unique_ptr'ı vektöre taşı
        // std::cout << "INFO: Entered Scope (Depth: " << newDepth << ")" << std::endl; // Debug
    }

    // Mevcut kapsamı kapatır
    void SymbolTable::exitScope() {
        // Global kapsamdan (depth 0) çıkmaya çalışıyor muyuz kontrol et.
        // Programın sonunda global kapsamdan çıkılır.
        assert(currentScope != nullptr && "Cannot exit scope when no scope is active.");

         std::cout << "INFO: Exiting Scope (Depth: " << currentScope->depth << ")" << std::endl; // Debug

        // currentScope'u üst kapsama taşı.
        currentScope = currentScope->parent;

        // scopes vektöründen son kapsamı sil. unique_ptr, objeyi otomatik temizler.
        scopes.pop_back();
    }

    // Mevcut kapsama bir sembol ekler.
    bool SymbolTable::insert(const std::string& name, std::shared_ptr<SymbolInfo> symbol) {
        assert(currentScope != nullptr && "Cannot insert symbol when no scope is active.");

        // Mevcut kapsamda aynı isimde sembol var mı kontrol et.
        if (currentScope->symbols.count(name)) {
            // Sembol zaten var, ekleme başarısız.
            // SEMA bu durumu hata olarak raporlamalıdır.
            return false;
        }

        // Sembolü mevcut kapsamın haritasına ekle.
        currentScope->symbols[name] = std::move(symbol); // shared_ptr'ı taşı
         std::cout << "INFO: Inserted Symbol '" << name << "' (" << getSymbolKindString(symbol->kind) << ") at Depth " << currentScope->depth << std::endl; // Debug
        return true;
    }

    // Bir ismi mevcut kapsamdan başlayarak üst kapsamlara doğru arar.
    std::shared_ptr<SymbolInfo> SymbolTable::lookup(const std::string& name) {
        Scope* current = currentScope;
        while (current != nullptr) {
            // Mevcut kapsamda ara
            auto it = current->symbols.find(name);
            if (it != current->symbols.end()) {
                // Bulundu, shared_ptr'ı döndür.
                 std::cout << "INFO: Looked up Symbol '" << name << "' (" << getSymbolKindString(it->second->kind) << ") found at Depth " << current->depth << std::endl; // Debug
                return it->second;
            }
            // Üst kapsama geç
            current = current->parent;
        }
         std::cout << "INFO: Looked up Symbol '" << name << "' - NOT FOUND" << std::endl; // Debug
        return nullptr; // Bulunamadı
    }

    // Bir ismi sadece mevcut kapsamda arar.
    std::shared_ptr<SymbolInfo> SymbolTable::lookupCurrentScope(const std::string& name) {
        assert(currentScope != nullptr && "Cannot lookup in current scope when no scope is active.");

        auto it = currentScope->symbols.find(name);
        if (it != currentScope->symbols.end()) {
              std::cout << "INFO: Looked up Symbol '" << name << "' (" << getSymbolKindString(it->second->kind) << ") found in current scope at Depth " << currentScope->depth << std::endl; // Debug
            return it->second;
        }
         std::cout << "INFO: Looked up Symbol '" << name << "' - NOT FOUND in current scope" << std::endl; // Debug
        return nullptr; // Bulunamadı
    }

    // Hata ayıklama için kapsam içeriğini yazdırır
    void SymbolTable::dumpScopes() const {
        std::cerr << "--- Symbol Table Scopes Dump ---" << std::endl;
        // Kapsamları en içten en dışa doğru (vektör sırasıyla) yazdıralım.
        for (const auto& scope_ptr : scopes) {
            const Scope& scope = *scope_ptr;
            std::cerr << "Scope Depth " << scope.depth << " (Parent Depth: " << (scope.parent ? scope.parent->depth : -1) << "):" << std::endl;
            if (scope.symbols.empty()) {
                std::cerr << "  (Empty)" << std::endl;
            } else {
                for (const auto& pair : scope.symbols) {
                    const std::string& name = pair.first;
                    const SymbolInfo& symbol = *pair.second;
                    std::cerr << "  - '" << name << "' [" << getSymbolKindString(symbol.kind)
                              << (symbol.isVariable() && symbol.isMutable ? ", mut" : "")
                              << "]: Type=" << (symbol.type ? symbol.type->toString() : "nullptr")
                              << (symbol.declarationNode ? " (Declared)" : " (Imported)")
                              << std::endl;
                }
            }
        }
        std::cerr << "--- End Symbol Table Scopes Dump ---" << std::endl;
    }

} // namespace cnt_compiler
