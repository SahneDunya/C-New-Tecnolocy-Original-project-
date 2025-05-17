#ifndef CNT_COMPILER_SYMBOL_TABLE_H
#define CNT_COMPILER_SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <memory>        // std::shared_ptr ve std::unique_ptr için
#include <unordered_map> // Kapsam içindeki sembolleri saklamak için

// Diğer bileşenlerden ileri bildirimler veya başlıklar
struct Type;           // TypeSystem'den gelecek semantik tip bilgisi için
struct DeclarationAST; // AST'deki bildirim düğümüne pointer için
struct TokenLocation;  // Konum bilgisi için (ast.h veya token.h'tan)

// Eğer Type ve DeclarationAST tanımları symbol_table.h'ı include eden başka dosyalardaysa,
// bu ileri bildirimler yeterli olabilir. Aksi halde ilgili başlıkları include edin.
#include "type_system.h" // Type* için (Type sınıfı tanımı burada olmalı)
#include "ast.h"         // TokenLocation, DeclarationAST* (ve alt sınıfları) için (ASTNode, DeclarationAST tanımları burada olmalı)


namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // Sembol türlerini ayırt etmek için enum
    enum class SymbolKind {
        Unknown,       // Bilinmeyen veya varsayılan
        Variable,      // Değişken (local veya global)
        Function,      // Fonksiyon veya metot
        Struct,        // Struct tanımı (tip ismi)
        Enum,          // Enum tanımı (tip ismi)
        EnumVariant,   // Enum varyantı (Match ifadesi, MemberAccess için kullanılabilir)
        StructField,   // Struct alanı (MemberAccess için kullanılabilir)
         Trait,
         AssociatedType,
        Module         // İmport edilen modüller için (alias durumunda)
    };

    // SymbolKind enum değerini string olarak döndüren yardımcı fonksiyon
    std::string getSymbolKindString(SymbolKind kind);

    // Bir sembolü temsil eden yapı (Değişken, Fonksiyon, Tip Adı vb.)
    struct SymbolInfo {
        std::string name;             // Sembolün ismi
        Type* type;                   // Anlamsal analiz sonrası çözülmüş tip bilgisi (TypeSystem tarafından yönetilen objeye pointer)
        SymbolKind kind;              // Sembol türü
        bool isMutable;               // Değiştirilebilir mi? (Değişkenler için geçerli)
        DeclarationAST* declarationNode; // Orijinal AST'deki bildirim düğümüne pointer (Import edilen veya intrincic semboller için nullptr)
        TokenLocation location;       // Tanımlandığı kaynak kod konumu (declarationNode varsa oradan alınır)

        // İmport edilen semboller veya arayüz temsili için gerekli ek bilgiler:
        // Bazı detaylar (Fonksiyon parametreleri, Struct alanları) tip bilgisinde (Type* içinde) zaten olabilir.
        // Ancak, parametre isimleri gibi sadece arayüzde gösterilecek bilgiler burada tutulabilir.
        // Örneğin, eğer FunctionType sadece tip pointer'ları tutuyorsa:
         std::vector<std::string> functionParameterNames; // Fonksiyon sembolleri için parametre isimleri
        // Struct/Enum sembolleri için varyant/alan isimleri ve detayları...

        // Constructor
        SymbolInfo(std::string n, Type* t, SymbolKind k, bool mut, DeclarationAST* decl, TokenLocation loc)
            : name(std::move(n)), type(t), kind(k), isMutable(mut), declarationNode(decl), location(loc) {}

         // Constructor for imported symbols (no declaration node)
         // Import edilen semboller için SymbolInfo oluştururken bu kullanılabilir.
         SymbolInfo(std::string n, Type* t, SymbolKind k, bool mut, TokenLocation loc)
            : name(std::move(n)), type(t), kind(k), isMutable(mut), declarationNode(nullptr), location(loc) {}

        // (Opsiyonel) Kolaylık sağlayan metodlar
        bool isVariable() const { return kind == SymbolKind::Variable; }
        bool isFunction() const { return kind == SymbolKind::Function; }
        // ... diğer türler
    };

    // Tek bir kapsamı (scope) temsil eden yapı
    // Kapsam, isimleri SymbolInfo objelerine haritalar.
    struct Scope {
        std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> symbols; // Bu kapsamdaki semboller
        Scope* parent; // Üst kapsama pointer (Global kapsam için nullptr)
        int depth; // Kapsamın iç içe geçme seviyesi (Global kapsam için 0)

        Scope(Scope* p, int d) : parent(p), depth(d) {}
    };

    // Sembol Tablosu Yönetim Sınıfı
    // Kapsamları yönetir (stack) ve sembol aramasını sağlar.
    class SymbolTable {
    private:
        // Kapsam yığını. unique_ptr, SymbolTable'ın Scope objelerinin sahipliğini yönetmesini sağlar.
        std::vector<std::unique_ptr<Scope>> scopes;
        Scope* currentScope; // Şu anki en üstteki kapsama pointer

    public:
        // Kurucu
        SymbolTable();
        // Yıkıcı (Bellek temizliği için, unique_ptr vectorü otomatik yapar)
        ~SymbolTable() = default;

        // Yeni bir kapsam açar (örn: fonksiyon girişi, blok girişi)
        void enterScope();

        // Mevcut kapsamı kapatır (örn: fonksiyon sonu, blok sonu)
        void exitScope();

        // Mevcut kapsam derinliğini döndürür (Global 0)
        int getCurrentScopeDepth() const { return currentScope ? currentScope->depth : -1; }

        // Mevcut kapsama bir sembol ekler.
        // Aynı isimde bir sembol zaten varsa false döner (veya hata raporlar). Başarılıysa true döner.
        bool insert(const std::string& name, std::shared_ptr<SymbolInfo> symbol);

        // Bir ismi mevcut kapsamdan başlayarak üst kapsamlara doğru arar.
        // Bulursa SymbolInfo'nun shared_ptr'ını, bulamazsa nullptr döner.
        std::shared_ptr<SymbolInfo> lookup(const std::string& name);

        // Bir ismi sadece mevcut kapsamda arar.
        // Bulursa SymbolInfo'nun shared_ptr'ını, bulamazsa nullptr döner.
        std::shared_ptr<SymbolInfo> lookupCurrentScope(const std::string& name);

        // Şu anki Scope objesine pointer döndürür (OwnershipChecker gibi bileşenler için faydalı)
        Scope* getCurrentScope() const { return currentScope; }

        // (Opsiyonel) Hata ayıklama için kapsam içeriğini yazdırır
        void dumpScopes() const;
    };

} // namespace cnt_compiler

#endif // CNT_COMPILER_SYMBOL_TABLE_H
