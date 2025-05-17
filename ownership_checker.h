#ifndef CNT_COMPILER_OWNERSHIP_CHECKER_H
#define CNT_COMPILER_OWNERSHIP_CHECKER_H

#include "ast.h" // AST düğümleri için
#include "diagnostics.h" // Hata raporlama için
#include "symbol_table.h" // Sembol bilgileri için (kim sahip, kim ödünç aldı vb.)
#include "type_system.h" // Tiplerin copy edilebilir olup olmadığı için

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

// İleri bildirimler
 struct ASTVisitor; // Visitor pattern kullanıyorsanız

// Değişkenin o anki durumunu temsil eden enum
enum VariableState {
    OWNED,        // Değişken değere sahip
    BORROWED_IMMUT, // Başkası tarafından immutable (&) ödünç alınmış
    BORROWED_MUT,   // Başkası tarafından mutable (&mut) ödünç alınmış
    MOVED,        // Değer başka yere taşınmış, değişken geçersiz
    PARTIALLY_MOVED, // Struct/enum gibi bir kısmının taşındığı durum
    COPIED        // Değer kopyalanabilir, değişken hala geçerli (Copy trait/özelliği olan tipler için)
    // ... Diğer durumlar (örn: Uninitialized)
};

// Ödünç alma bilgisi
struct BorrowInfo {
    ASTNode* borrowerNode; // Ödünç alan ifadenin AST düğümü
    TokenLocation borrowLocation; // Ödünç almanın yapıldığı kaynak kod konumu
    bool isMutable;       // &mut ödünç alma mı?
    // Lifetime bilgisi burada veya ayrı bir yapıda tutulabilir.
     void* lifetime; // Yaşam süresini temsil eden bir ID veya pointer
};

// Değişken veya değer için sahiplik ve ödünç alma durumu
struct ValueStatus {
    VariableState state = OWNED; // Varsayılan durum
    std::vector<BorrowInfo> activeBorrows; // O anki aktif ödünç almalar
    ASTNode* lastUseNode = nullptr; // Değerin en son kullanıldığı AST düğümü (Move tespiti için)
    TokenLocation declarationLocation; // Tanımlandığı konum
    bool isCopyType = false; // Bu tip kopyalanabilir mi?

    // Varsayılan kurucu
    ValueStatus() = default;

     // Bildirimden oluştururken
    ValueStatus(TokenLocation declLoc, bool copyType)
        : declarationLocation(declLoc), isCopyType(copyType) {}

    // Belirli bir noktadaki durumu kopyala (snapshot)
    ValueStatus clone() const {
        ValueStatus copy = *this;
        // activeBorrows'u da kopyala
        return copy;
    }
};


// Sahiplik ve Ödünç Alma Kurallarını Kontrol Eden Sınıf
class OwnershipChecker {
private:
    Diagnostics& diagnostics; // Hata raporlama
    TypeSystem& typeSystem;   // Tip bilgisi (Copy trait vb.)

    // Değişkenlerin/değerlerin durumunu takip eden harita
    // AST düğümlerine (VarDeclAST) veya SymbolInfo'ya karşılık gelebilir.
    // Karmaşık durumlar için ValueStatus'ı SymbolInfo'ya eklemek veya ayrı bir harita kullanmak gerekebilir.
    // Örneğin: FunctionArgAST'ler, return değerleri, temporary değerler de sahiplik durumuna sahip olabilir.
    // Şimdilik basitçe VarDeclAST* -> ValueStatus eşlemesi yapalım.
    std::unordered_map<const VarDeclAST*, ValueStatus> variableStates;

    // Traversal sırasında scope'a özgü borrow/ownership state'ini tutmak gerekebilir
    // Örneğin, bir bloktan çıkarken ödünç almaların geçerliliği biter.
     std::vector<std::unordered_map<const VarDeclAST*, ValueStatus>> scopeStates; // Kapsam yığını gibi

    // Yaşam süresi bilgisi (İşaretçiler/referanslar için)
    // AST düğümüne (& veya &mut UnaryOpAST) karşılık gelen yaşam süresi ID/pointer
     std::unordered_map<const UnaryOpAST*, void*> lifetimeMap; // void* yerine uygun bir lifetime yapısı

    // Yardımcı analiz metodları (AST düğümlerini gezerken çağrılır)
    void analyzeNode(ASTNode* node);
    void analyzeStatement(StatementAST* stmt);
    void analyzeExpression(ExpressionAST* expr);
    void analyzeVarDecl(VarDeclAST* varDecl);
    void analyzeAssignment(AssignmentAST* assign);
    void analyzeCallExpression(CallExpressionAST* call);
    void analyzeUnaryOp(UnaryOpAST* unaryOp); // '&', '&mut', '*' işlemleri için
    void analyzeIdentifier(IdentifierAST* identifier); // Değişken kullanımları için (move/borrow)
    // ... Diğer AST düğümleri için analiz metodları (match, loop, if vb.)

    // Kuralları kontrol eden metodlar
    void checkMove(const VarDeclAST* sourceVar, TokenLocation useLoc); // Değer taşındıktan sonra kullanımı kontrol et
    void recordBorrow(const VarDeclAST* borrowedVar, const BorrowInfo& borrow); // Bir değişkenden ödünç alma kaydı yap
    void checkBorrowValidity(const VarDeclAST* usedVar, TokenLocation useLoc); // Ödünç alınan değerin kullanımını kontrol et
    void endBorrowsInScope(Scope* scope); // Kapsam dışına çıkan ödünç almaların geçerliliğini sonlandır
    void checkMutability(const VarDeclAST* var, TokenLocation useLoc, bool isWriteOperation); // Değiştirilebilirlik kontrolü

    // Yaşam süresi analizi ve kontrolü (Karmaşık bir konudur)
    // Referansların gösterdiği değerlerin yaşam süresi içinde kullanıldığını doğrulamak
     void analyzeLifetimes(ASTNode* node); // AST üzerinde yaşam sürelerini çıkar veya doğrula

public:
    OwnershipChecker(Diagnostics& diag, TypeSystem& ts);

    // AST'yi analiz et ve sahiplik/ödünç alma/yaşam süresi kurallarını kontrol et
    bool check(ProgramAST* program); // true hata varsa, false yoksa

    // Belirli bir değişkenin durumunu al (hata ayıklama için)
    VariableState getVariableState(const VarDeclAST* varDecl) const;
};

#endif // CNT_COMPILER_OWNERSHIP_CHECKER_H
