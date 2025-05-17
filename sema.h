#ifndef CNT_COMPILER_SEMA_H
#define CNT_COMPILER_SEMA_H

// Temel AST başlığı (diğer AST başlıklarını da içerir)
#include "ast.h"
#include "expressions.h"
#include "statements.h"
#include "declarations.h"
#include "types.h"

// Yardımcı sistem başlıkları
#include "diagnostics.h"
#include "symbol_table.h"
#include "type_system.h"
#include "ownership_checker.h"
#include "module_resolver.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// Visitor pattern kullanmıyorsak, AST düğümlerini gezen ve analiz eden metodlar
// (Parser'daki parse metotlarına benzer bir yapı, ama anlamsal kontrol yapar)

class SemanticAnalyzer {
private:
    Diagnostics& diagnostics;         // Hata raporlama sistemi
    SymbolTable symbolTable;          // Kapsam ve sembol yönetimi
    TypeSystem typeSystem;            // Semantik tip temsilleri ve kontrolleri
    OwnershipChecker ownershipChecker; // Sahiplik, ödünç alma, yaşam süresi kuralları
    ModuleResolver moduleResolver;    // Modül ve import çözümlemesi

    // Traversal sırasında ihtiyaç duyulan durum bilgileri
    Type* currentFunctionReturnType = nullptr; // Şu an analiz edilen fonksiyonun dönüş tipi
    bool inLoop = false;                       // Döngü içinde miyiz? (break/continue kontrolü için)
    bool inFunction = false;                   // Fonksiyon içinde miyiz? (return kontrolü için)
    // ... Diğer durum bilgileri (örn: struct/enum/trait içinde miyiz?)

    // =======================================================================
    // AST Düğümlerini Gezen ve Analiz Eden Metodlar
    // Bu metodlar AST yapısına göre birbirini özyinelemeli (recursive) çağırır.
    // =======================================================================

    // Genel düğüm analizcisi (switch/if-else ile türüne göre dallanabilir)
    void analyzeNode(ASTNode* node);

    // Bildirim analizcileri
    void analyzeProgram(ProgramAST* program);
    void analyzeDeclaration(DeclarationAST* decl); // Türüne göre alt analizcilere dallanır
    void analyzeFunctionDecl(FunctionDeclAST* funcDecl);
    void analyzeStructDecl(StructDeclAST* structDecl);
    void analyzeEnumDecl(EnumDeclAST* enumDecl);
    void analyzeVarDecl(VarDeclAST* varDecl, bool isGlobal); // Global/Yerel değişken ayrımı yapılabilir

    // Deyim analizcileri
    void analyzeStatement(StatementAST* stmt); // Türüne göre alt analizcilere dallanır
    void analyzeBlockStatement(BlockStatementAST* block);
    void analyzeImportStatement(ImportStatementAST* importStmt);
    void analyzeReturnStatement(ReturnStatementAST* returnStmt);
    void analyzeBreakStatement(BreakStatementAST* breakStmt);
    void analyzeContinueStatement(ContinueStatementAST* continueStmt);
    void analyzeWhileStatement(WhileStatementAST* whileStmt);
     void analyzeIfStatement(IfStatementAST* ifStmt); // Eğer if/else destekleniyorsa
     void analyzeForStatement(ForStatementAST* forStmt); // Eğer for destekleniyorsa


    // İfade analizcileri (İfadenin semantik tipini döndürür)
    Type* analyzeExpression(ExpressionAST* expr); // Türüne göre alt analizcilere dallanır
    Type* analyzeIntLiteral(IntLiteralAST* literal);
    Type* analyzeFloatLiteral(FloatLiteralAST* literal);
    Type* analyzeStringLiteral(StringLiteralAST* literal);
    Type* analyzeCharLiteral(CharLiteralAST* literal);
    Type* analyzeBoolLiteral(BoolLiteralAST* literal);
    Type* analyzeIdentifier(IdentifierAST* identifier); // İsim çözümlemesi ve tipini bulma
    Type* analyzeBinaryOp(BinaryOpAST* binaryOp);     // Operatör aşırı yüklemesi (overloading) ve tip uyumluluğu
    Type* analyzeUnaryOp(UnaryOpAST* unaryOp);       // Tekli operatörler ve tip uyumluluğu (&, &mut, *)
    Type* analyzeAssignment(AssignmentAST* assignment); // Atama kuralları (atanabilirlik, mutability)
    Type* analyzeCallExpression(CallExpressionAST* call); // Fonksiyon/metot çözümlemesi ve argüman tip kontrolü
    Type* analyzeMemberAccess(MemberAccessAST* memberAccess); // Üye çözümlemesi ve tipini bulma
    Type* analyzeIndexAccess(IndexAccessAST* indexAccess); // Index tipinin kontrolü
    Type* analyzeMatchExpression(MatchExpressionAST* matchExpr); // Match ifadesi, pattern ve kolların analizi

    // Tip AST düğümlerini semantik tiplere dönüştüren analizciler
    Type* analyzeTypeAST(TypeAST* typeNode); // Türüne göre alt analizcilere dallanır
    Type* analyzeBaseTypeAST(BaseTypeAST* baseTypeNode); // İsim çözümlemesi yapıp Struct/Enum tipine dönüştürebilir
    Type* analyzeReferenceTypeAST(ReferenceTypeAST* refTypeNode);
    Type* analyzeArrayTypeAST(ArrayTypeAST* arrayTypeNode);
    // ... Diğer TypeAST türleri için analizciler

    // =======================================================================
    // Anlamsal Kontrol ve Doğrulama Yardımcı Metodları
    // =======================================================================

    // İki semantik tipin eşit olup olmadığını kontrol et
    bool areTypesEqual(Type* t1, Type* t2);
    // Bir tipin diğerine atanabilir olup olmadığını kontrol et (Mutability dikkate alınır)
    bool isAssignable(Type* valueType, Type* targetType, bool isTargetMutable);

    // Fonksiyon çağrısının doğru olup olmadığını kontrol et (argüman sayısı/tipi)
    void checkFunctionCall(CallExpressionAST* callExpr, FunctionDeclAST* targetFunc);

    // Match ifadesi için kapsamlılık ve erişilebilirlik kontrolü
    void checkMatchExhaustiveness(MatchExpressionAST* matchExpr, Type* valueType); // Tüm durumlar ele alınmış mı?
    void checkMatchReachability(MatchExpressionAST* matchExpr); // Ulaşılamayan kollar var mı?

    // Sahiplik, Ödünç Alma, Yaşam Süresi kurallarını uygula
    // Bu metodlar OwnershipChecker sınıfının metodlarını çağıracaktır.
    void enforceOwnershipRules(ASTNode* node); // Genel çağrı
    // ... Belirli durumlar için daha detaylı çağrılar (örn: checkBorrow(expr), recordMove(vardecl))


    // AST düğümlerine semantik bilgi ekle (Annotations/Decorations)
    // Örneğin, bir IdentifierAST düğümüne çözümlenmiş SymbolInfo* veya Type* pointer'ı ekleyebiliriz.
    // AST düğümlerinizin bu bilgileri tutacak üyelere sahip olması gerekir.
     void attachTypeInfo(ExpressionAST* expr, Type* type);
     void attachDeclarationInfo(IdentifierAST* id, SymbolInfo* symbol);


public:
    // Kurucu: Yardımcı sistemlerin referanslarını veya pointerlarını alır
    SemanticAnalyzer(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, OwnershipChecker& oc, ModuleResolver& mr);

    // Semantik analizi başlatan ana metod
    // true dönerse anlamsal hatalar bulundu demektir.
    bool analyze(ProgramAST* program);
};

#endif // CNT_COMPILER_SEMA_H
