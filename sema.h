#ifndef CNT_COMPILER_SEMA_H
#define CNT_COMPILER_SEMA_H

// Güncellenmiş AST başlıkları (yeni üyelerle)
#include "ast.h"
#include "expressions.h"
#include "statements.h"
#include "declarations.h"
#include "types.h"

// Yardımcı sistem başlıkları
#include "diagnostics.h"
#include "symbol_table.h"      // SymbolInfo, SymbolTable
#include "type_system.h"       // Type, TypeSystem
#include "ownership_checker.h" // OwnershipChecker
#include "module_manager.h"    // ModuleResolver, ModuleInterface

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>


// Semantic Analyzer sınıfı
class SemanticAnalyzer {
private:
    Diagnostics& diagnostics;         // Hata raporlama sistemi
    TypeSystem& typeSystem;           // Semantik tip temsilleri ve kontrolleri
    SymbolTable& symbolTable;         // Kapsam ve sembol yönetimi
    OwnershipChecker& ownershipChecker; // Sahiplik, ödünç alma, yaşam süresi kuralları
    ModuleResolver& moduleResolver;   // Modül ve import çözümlemesi

    // Traversal sırasında ihtiyaç duyulan durum bilgileri
    Type* currentFunctionReturnType = nullptr; // Şu an analiz edilen fonksiyonun dönüş tipi
    bool inLoop = false;                       // Döngü içinde miyiz? (break/continue kontrolü için)
    bool inFunction = false;                   // Fonksiyon içinde miyiz? (return kontrolü için)
    // ... Diğer durum bilgileri (örn: struct/enum/trait içinde miyiz?)

    // =======================================================================
    // AST Düğümlerini Gezen ve Analiz Eden Metodlar
    // Bu metodlar AST yapısına göre birbirini özyinelemeli (recursive) çağırır.
    // Expression üreten metodlar Type* döndürür ve AST düğümüne resolvedSemanticType'ı yazar.
    // Statement ve Declaration üreten metodlar genellikle void döner (resolvedSymbol/Type'ları AST'ye yazar).
    // =======================================================================

    // Genel düğüm analizcisi (switch/if-else ile türüne göre dallanabilir)
    void analyzeNode(ASTNode* node);

    // Bildirim analizcileri
    void analyzeProgram(ProgramAST* program);
    void analyzeDeclaration(DeclarationAST* decl); // Türüne göre alt analizcilere dallanır, isPublic'i kontrol eder
    void analyzeFunctionDecl(FunctionDeclAST* funcDecl); // resolvedSymbol, resolvedSemanticType'ı ayarlar
    void analyzeStructDecl(StructDeclAST* structDecl);   // resolvedSymbol, resolvedSemanticType'ı ayarlar
    void analyzeEnumDecl(EnumDeclAST* enumDecl);     // resolvedSymbol, resolvedSemanticType'ı ayarlar
    void analyzeVarDecl(VarDeclAST* varDecl); // resolvedSymbol, resolvedSemanticType'ı ayarlar (initializer'dan)

    // Deyim analizcileri
    void analyzeStatement(StatementAST* stmt); // Türüne göre alt analizcilere dallanır
    void analyzeBlockStatement(BlockStatementAST* block); // Kapsam girer/çıkar
    void analyzeImportStatement(ImportStatementAST* importStmt); // ModuleResolver'ı çağırır, resolvedInterface'i ayarlar
    void analyzeReturnStatement(ReturnStatementAST* returnStmt);
    void analyzeBreakStatement(BreakStatementAST* breakStmt);
    void analyzeContinueStatement(ContinueStatementAST* continueStmt);
    void analyzeWhileStatement(WhileStatementAST* whileStmt);
     void analyzeIfStatement(IfStatementAST* ifStmt);
     void analyzeForStatement(ForStatementAST* forStmt);


    // İfade analizcileri (İfadenin semantik tipini döndürür ve AST düğümüne yazar)
    Type* analyzeExpression(ExpressionAST* expr); // Türüne göre alt analizcilere dallanır, resolvedSemanticType'ı ayarlar
    Type* analyzeIntLiteral(IntLiteralAST* literal); // resolvedSemanticType'ı ayarlar (IntType)
    Type* analyzeFloatLiteral(FloatLiteralAST* literal); // resolvedSemanticType'ı ayarlar (FloatType)
    Type* analyzeStringLiteral(StringLiteralAST* literal); // resolvedSemanticType'ı ayarlar (StringType)
    Type* analyzeCharLiteral(CharLiteralAST* literal);   // resolvedSemanticType'ı ayarlar (CharType)
    Type* analyzeBoolLiteral(BoolLiteralAST* literal);   // resolvedSemanticType'ı ayarlar (BoolType)
    Type* analyzeIdentifier(IdentifierAST* identifier); // İsim çözümlemesi yapar, resolvedSymbol, resolvedSemanticType'ı ayarlar
    Type* analyzeBinaryOp(BinaryOpAST* binaryOp);     // Operandları analiz eder, tip uyumluluğunu kontrol eder, resolvedSemanticType'ı ayarlar
    Type* analyzeUnaryOp(UnaryOpAST* unaryOp);       // Operandı analiz eder, tip uyumluluğunu kontrol eder, resolvedSemanticType'ı ayarlar (&,* için ownership/borrowing çağrısı)
    Type* analyzeAssignment(AssignmentAST* assignment); // Sol/sağ tarafı analiz eder, atanabilirlik/mutability kontrolü, resolvedSemanticType'ı ayarlar
    Type* analyzeCallExpression(CallExpressionAST* call); // Callee/argümanları analiz eder, resolvedCalleeSymbol, resolvedSemanticType'ı ayarlar, argüman tip kontrolü
    Type* analyzeMemberAccess(MemberAccessAST* memberAccess); // Base/member'ı analiz eder, resolvedMemberSymbol, resolvedSemanticType'ı ayarlar
    Type* analyzeIndexAccess(IndexAccessAST* indexAccess); // Base/index'i analiz eder, resolvedSemanticType'ı ayarlar
    Type* analyzeMatchExpression(MatchExpressionAST* matchExpr); // Value/arms'ı analiz eder, kapsamlılık/erişilebilirlik kontrolü, resolvedSemanticType'ı ayarlar

    // Tip AST düğümlerini analiz eder (Semantik tip objesine pointer döndürür ve AST düğümüne yazar)
    Type* analyzeTypeAST(TypeAST* typeNode); // Türüne göre alt analizcilere dallanır, resolvedSemanticType'ı ayarlar
    Type* analyzeBaseTypeAST(BaseTypeAST* baseTypeNode); // İsim çözümlemesi yapıp TypeSystem'den Type* alır, resolvedSemanticType'ı ayarlar
    Type* analyzeReferenceTypeAST(ReferenceTypeAST* refTypeNode); // resolvedSemanticType'ı ayarlar
    Type* analyzeArrayTypeAST(ArrayTypeAST* arrayTypeNode);     // resolvedSemanticType'ı ayarlar
    // ... Diğer TypeAST türleri için analizciler

    // =======================================================================
    // Anlamsal Kontrol ve Doğrulama Yardımcı Metodları
    // =======================================================================

    // İki semantik tipin eşit olup olmadığını kontrol et (TypeSystem kullanır)
    bool areTypesEqual(Type* t1, Type* t2);
    // Bir tipin diğerine atanabilir olup olmadığını kontrol et (Mutability dikkate alınır) (TypeSystem kullanır)
    bool isAssignable(Type* valueType, Type* targetType, bool isTargetMutable);

    // Fonksiyon çağrısının doğru olup olmadığını kontrol et (argüman sayısı/tipi)
    void checkFunctionCall(CallExpressionAST* callExpr, SymbolInfo* calleeSymbol); // Callee SymbolInfo'su ile kontrol

    // Match ifadesi için kapsamlılık ve erişilebilirlik kontrolü (OwnershipChecker veya ayrı helper)
    void checkMatchExhaustiveness(MatchExpressionAST* matchExpr, Type* valueType); // Tüm durumlar ele alınmış mı?
    void checkMatchReachability(MatchExpressionAST* matchExpr); // Ulaşılamayan kollar var mı?

    // Belirli bir bildirim türünün public olmasına izin verilip verilmediğini kontrol et
    bool isValidPublicDeclarationType(const ASTNode* node) const;


    // Sahiplik, Ödünç Alma, Yaşam Süresi kurallarını uygula (OwnershipChecker'ı çağırır)
     void enforceOwnershipRules(ASTNode* node); // Genel çağrı
    // ... Belirli durumlar için daha detaylı çağrılar


public:
    // Kurucu: Yardımcı sistemlerin referanslarını alır
    SemanticAnalyzer(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, OwnershipChecker& oc, ModuleResolver& mr);

    // Semantik analizi başlatan ana metod
    // true dönerse anlamsal hatalar bulundu demektir.
    bool analyze(ProgramAST* program);
};

#endif // CNT_COMPILER_SEMA_H
