#ifndef CNT_COMPILER_SYMBOL_TABLE_H
#define CNT_COMPILER_SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory> // std::shared_ptr veya ham pointer kullanabilirsiniz

// İleri bildirimler
struct Type; // Tip sisteminizdeki semantik tip temsili
struct DeclarationAST; // Deklarasyon AST düğümüne referans tutabilir

// Sembol bilgisi
struct SymbolInfo {
    std::string name;
    Type* type; // Semantik tip sistemi objesine pointer
    DeclarationAST* declarationNode; // Bu sembolün tanımlandığı AST düğümüne pointer
    bool isMutable; // Değişken ise değiştirilebilir mi?
    // ... diğer özellikler (örn: kapsam, erişim belirleyici, fonksiyon parametresi mi vb.)

    SymbolInfo(std::string n, Type* t, DeclarationAST* decl, bool mut)
        : name(std::move(n)), type(t), declarationNode(decl), isMutable(mut) {}
};

// Kapsam (Scope) seviyesi
struct Scope {
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> symbols; // İsimden sembol bilgisine eşleme
     std::weak_ptr<Scope> parent; // Üst kapsama geri referans (isteğe bağlı)
};

// Sembol Tablosu sınıfı
class SymbolTable {
private:
    std::vector<std::shared_ptr<Scope>> scopes; // Kapsam yığını

public:
    SymbolTable();

    // Yeni bir kapsam oluştur ve gir
    void enterScope();

    // Mevcut kapsamdan çık
    void exitScope();

    // Mevcut kapsama sembol ekle
    bool insert(const std::string& name, std::shared_ptr<SymbolInfo> symbol); // true başarı, false zaten var

    // İsim çözümlemesi: mevcut ve üst kapsamlarda sembolü ara
    std::shared_ptr<SymbolInfo> lookup(const std::string& name);

    // Sadece mevcut kapsamda sembolü ara
    std::shared_ptr<SymbolInfo> lookupCurrentScope(const std::string& name);
};

#endif // CNT_COMPILER_SYMBOL_TABLE_H
