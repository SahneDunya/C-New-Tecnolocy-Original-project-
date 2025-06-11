#ifndef CNT_COMPILER_OWNERSHIP_CHECKER_H
#define CNT_COMPILER_OWNERSHIP_CHECKER_H

#include "diagnostics.h" // Hata raporlama
#include "type_system.h" // Tip Sistemi (Copy, Drop, tipler)
#include "symbol_table.h" // Sembol Tablosu (SymbolInfo, Scope)
#include "ast.h"         // AST düğümleri (TokenLocation, ASTNode)
// AST düğümlerinin alt sınıfları da gerekebilir (FunctionCallAST, UnaryOpAST vb.)
#include "expressions.h"
#include "statements.h"
#include "declarations.h"


#include <unordered_map> // Durum takibi için
#include <vector>
#include <string>
#include <memory>        // shared_ptr için (SymbolInfo)


namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // Bir değişkenin anlık durumunu temsil eden enum
    enum class VariableStatus {
        Owned,           // Değişken değere sahip
        Moved,           // Değer başka bir yere taşındı (artık geçerli değil)
        BorrowedImmutable, // Değişkene şu anda & referansı var
        BorrowedMutable, // Değişkene şu anda &mut referansı var
        Dropped,         // Değişken kapsamdan çıktı ve değeri Drop edildi
        Uninitialized,   // Değişken tanımlandı ama değeri atanmadı (SEMA da kontrol eder)
         PartiallyMoved // Struct alanları gibi kısmi taşıma için (Daha gelişmiş)
    };

    // Bir değişken kullanımının türünü temsil eden enum
    enum class UseKind {
        Read,           // Değer okunuyor (örn: x + 1)
        Write,          // Değer değiştiriliyor (örn: x = 5)
        Move,           // Değer sahipliği taşınıyor (örn: func(x), y = x)
        BorrowImmutable, // &x referansı alınıyor
        BorrowMutable    // &mut x referansı alınıyor
         CallAsMethod, // Metot çağrısı (self referansını içerir)
    };

    // Active olan bir ödünç almayı (borrow) temsil eden yapı
    struct ActiveBorrow {
        const ASTNode* sourceNode;    // Bu ödünç almayı oluşturan AST düğümü (&x, &mut y ifadesi)
        TokenLocation location; // Ödünç alma işleminin kaynak kod konumu
        bool isMutable;         // Mutable (&mut) mı, Immutable (&) mı?
        const Scope* borrowScope;     // Ödünç almanın geçerli olduğu kapsam (Yaşam süresinin basit bir gösterimi)

        // Ödünç alınan değerin kaynağına işaret eden bilgi
        // Bu SymbolInfo* bir değişkene işaret edebilir.
        const SymbolInfo* borrowedSymbol; // Ödünç alınan değişken (eğer direkt değişken ise)
        // Daha karmaşık durumlar (alan veya dizi elemanı ödünç alma) için ek bilgi gerekebilir.
         const ASTNode* borrowedTargetNode; // Ödünç alınan ifadenin AST düğümü (örn: obj.field, arr[i])

        // Kurucu
        ActiveBorrow(const ASTNode* src_node, TokenLocation loc, bool mut, const Scope* scope, const SymbolInfo* symbol)
            : sourceNode(src_node), location(loc), isMutable(mut), borrowScope(scope), borrowedSymbol(symbol) {}

        // Ödünç almanın bir diğeriyle çakışıp çakışmadığını kontrol et (Temel kural: tek mut veya çok immut)
        bool conflictsWith(const ActiveBorrow& other) const {
            // Mutable ödünç alma, başka herhangi bir ödünç alma ile çakışır.
            if (isMutable || other.isMutable) return true;
            // İki immutable ödünç alma çakışmaz.
            return false;
        }

        // Bir ödünç almanın belirli bir kullanımla çakışıp çakışmadığını kontrol et
        bool conflictsWithUse(UseKind useKind) const {
            if (isMutable) return true; // Mutable ödünç alma her türlü kullanımla çakışır (kendi kullanımı hariç)
            if (useKind == UseKind::Write || useKind == UseKind::Move || useKind == UseKind::BorrowMutable) return true; // Immutable ödünç alma, mutable kullanımla çakışır
            return false; // Immutable ödünç alma, read veya immutable borrow ile çakışmaz
        }
    };


    // Sahiplik ve Ödünç Alma Kurallarını Takip Eden ve Uygulayan Sınıf
    class OwnershipChecker {
    private:
        Diagnostics& diagnostics;    // Referans
        TypeSystem& typeSystem;      // Referans (Copy/Drop trait bilgisi için)
        SymbolTable& symbolTable;    // Referans (Kapsam ve Sembol bilgisi için)

        // Durum Takibi:
        // Değişkenlerin anlık durumunu takip eder (Owned, Moved, Borrowed vb.)
        // SymbolInfo* -> Anlık Durum
        std::unordered_map<const SymbolInfo*, VariableStatus> variableStatuses;

        // Aktif ödünç almaları takip eder.
        // Key: Hangi değerin ödünç alındığı (örn: Değişkenin SymbolInfo*'ı, Struct Alanı referansı, Dizi Elemanı referansı)
        // Value: O değere ait aktif ödünç almaların listesi (Vector<ActiveBorrow>)
        // Basitlik için şimdilik sadece değişkenlere ait ödünç almaları takip edelim.
        std::unordered_map<const SymbolInfo*, std::vector<ActiveBorrow>> activeVariableBorrows;

        // Kapsam yığını (SymbolTable'ın kapsamlarına pointerlar) ödünç alma yaşam sürelerini yönetmek için.
        // enterScope/exitScope ile senkronize edilir.
        std::vector<const Scope*> scopeStack;


        // =======================================================================
        // Internal Helper Methods
        // =======================================================================

        // Bir tipin Copy trait'ini implemente edip etmediğini kontrol et
        // TypeSystem'e dayanır. Temel tipler Copy'dir, Struct/Enum alanları Copy ise Copy'dir, Referanslar Copy'dir.
        bool implementsCopy(Type* type) const;

        // Bir tipin Drop implementasyonu olup olmadığını kontrol et (Resource yönetimi)
        // TypeSystem'e dayanır.
        bool hasDrop(Type* type) const;

        // Bir sahiplik/ödünç alma hatası raporla
        void reportOwnershipError(const TokenLocation& location, const std::string& message);

        // Ödünç alma çakışmasını raporla (Çakışan ödünç alma hakkında bilgi içerir)
        void reportBorrowConflict(const TokenLocation& newBorrowLocation, bool isNewBorrowMutable, const SymbolInfo* borrowedSymbol, const ActiveBorrow& conflictingBorrow);

        // Belirli bir kapsam dışına çıkan tüm aktif ödünç almaları kaldır
        void endBorrowsInScope(const Scope* scopeToEnd);

        // Bir değişkenin mevcut durumunun belirli bir kullanım türüne izin verip vermediğini kontrol et
        bool canUseVariable(const SymbolInfo* symbol, UseKind useKind, const TokenLocation& location) const;

        // Bir ifade sonucunun (geçici değer) sahiplik/copy/drop durumunu yönet.
        void handleExpressionResult(ExpressionAST* expr);


    public:
        // Kurucu
        OwnershipChecker(Diagnostics& diag, TypeSystem& ts, SymbolTable& st);

        // =======================================================================
        // Semantic Analyzer tarafından AST Traversal Sırasında Çağrılan Metodlar
        // SEMA, AST düğümlerini analiz ederken bu metodları uygun yerlerde çağırır.
        // =======================================================================

        // Yeni bir kapsam açılırken çağrılır
        void enterScope(const Scope* scope);

        // Mevcut kapsam kapanırken çağrılır (Kaynakların drop edildiği yer)
        void exitScope();

        // Bir değişken bildirimi analiz edilirken çağrılır (let x = ...;)
        void recordVariableDeclaration(const SymbolInfo* symbol, const TokenLocation& location);

        // Bir değişkenin kullanımı analiz edilirken çağrılır (x + 1, x = 5, func(x))
        // Kullanımın türü (okuma, yazma, taşıma vb.) belirtilir.
        void handleVariableUse(const SymbolInfo* symbol, UseKind useKind, const TokenLocation& location, const ASTNode* useNode);

        // Atama ifadesi analiz edilirken çağrılır (sol = sag)
        void handleAssignment(const ExpressionAST* leftExpr, const ExpressionAST* rightExpr, const TokenLocation& location);

        // Fonksiyon çağrısı analiz edilirken çağrılır (Argümanlar ve dönüş değeri)
        void handleFunctionCall(const CallExpressionAST* callExpr);

        // Referans oluşturma (&expr veya &mut expr) analiz edilirken çağrılır
        void handleReferenceCreation(const UnaryOpAST* unaryOp, const SymbolInfo* borrowedSymbol); // borrowedSymbol, ödünç alınan değişkenin sembolü olabilir.

        // Dereference (*expr) analiz edilirken çağrılır
        void handleDereference(const UnaryOpAST* unaryOp);

        // Return deyimi analiz edilirken çağrılır
        void handleReturn(const ReturnStatementAST* returnStmt);

        // Break veya Continue deyimi analiz edilirken çağrılır
        void handleJump(const StatementAST* jumpStmt); // BreakStatementAST veya ContinueStatementAST

        // Match kollarının veya If/Else dallarının birleştiği yerde çağrılabilir
        // Durumun birleştirilmesi gereken karmaşık bir analiz gerektirir.
        void handleBranchMerge(const TokenLocation& mergeLocation);
    };

} // namespace cnt_compiler

#endif // CNT_COMPILER_OWNERSHIP_CHECKER_H
