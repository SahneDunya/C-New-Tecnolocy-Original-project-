#ifndef CNT_COMPILER_CODEGEN_H
#define CNT_COMPILER_CODEGEN_H

// AST başlıkları
#include "ast.h"
#include "expressions.h"
#include "statements.h"
#include "declarations.h"
#include "types.h"

// Anlamsal Analiz çıktıları
#include "diagnostics.h"   // Hata raporlama (kod üretimi hataları semantik hatalara göre daha az olmalı)
#include "symbol_table.h"  // İsim çözümleme bilgileri
#include "type_system.h"   // Semantik tip bilgileri

// LLVM Başlıkları
#include <llvm/IR/IRBuilder.h>      // LLVM IR komutları oluşturmak için ana sınıf
#include <llvm/IR/LLVMContext.h>    // LLVM ortamı için ana konteyner
#include <llvm/IR/Module.h>         // Tüm kodun LLVM temsili
#include <llvm/IR/Value.h>          // LLVM IR'deki tüm değerlerin (değişkenler, sabitler, fonksiyonlar, komutlar) temel sınıfı
#include <llvm/IR/Type.h>           // LLVM IR'deki tüm tiplerin temel sınıfı
#include <llvm/Target/TargetMachine.h> // Hedef mimariye özgü bilgiler
#include <llvm/IR/Constants.h>      // Sabit değerler oluşturmak için
#include <llvm/IR/Verifier.h>       // Üretilen LLVM IR'yi doğrulamak için (debug amaçlı)
#include <llvm/ADT/APInt.h>         // Arbitrary Precision Int - Sabit tamsayılar için
#include <llvm/ADT/APFloat.h>       // Arbitrary Precision Float - Sabit ondalıklı sayılar için

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// İleri bildirimler (LLVM namespace'i içindekiler)
namespace llvm {
    class Function;
    class BasicBlock;
    class AllocaInst; // Yerel değişken ayırma komutu
}


// Kod üretimi sırasında geçici state veya haritalamaları tutacak bağlam (context)
struct CodeGenContext {
    // Semantik sembol bilgisi ile LLVM IR'deki tahsis edilen yer (AllocaInst) veya değer arasına haritalama
    // Yerel değişkenler için kullanılır. Global değişkenler LLVM Module içinde doğrudan bulunur.
    std::unordered_map<const SymbolInfo*, llvm::AllocaInst*> variableAllocations;

    // Semantik fonksiyon bilgisi ile LLVM Function* arasına haritalama
    std::unordered_map<const SymbolInfo*, llvm::Function*> functionMap;

    // Şu anda üretilen fonksiyon (break, continue gibi kontrol akışı için gerekli)
    llvm::Function* currentFunction = nullptr;

    // Döngüler için break ve continue hedefleri (LLVM BasicBlock*)
    std::vector<llvm::BasicBlock*> breakTargets;    // En içteki döngünün break hedefi yığının en üstünde
    std::vector<llvm::BasicBlock*> continueTargets; // En içteki döngünün continue hedefi yığının en üstünde

    // ... Diğer bağlam bilgileri (örn: mevcut struct, trait vb.)
};


// LLVM Kod Üretici Sınıfı
class LLVMCodeGenerator {
private:
    Diagnostics& diagnostics;   // Hata raporlama (burada nadir olmalı)
    TypeSystem& typeSystem;     // Semantik tip sistemi (semantic Type* objelerini LLVM Type*'a çevirmek için)
    SymbolTable& symbolTable;   // Sembol tablosu (çözümlenmiş sembol bilgilerini almak için)

    llvm::LLVMContext& context; // LLVM bağlamı (tüm LLVM objeleri bu bağlamda yaşar)
    std::unique_ptr<llvm::Module> module; // Üzerinde çalıştığımız LLVM modülü
    llvm::IRBuilder<> builder;   // IR komutları oluşturan ana yardımcı
    llvm::TargetMachine& targetMachine; // Hedef mimari bilgileri

    CodeGenContext codegenContext; // Kod üretimi bağlamı

    // Semantik tip (Type*) ile LLVM tip (llvm::Type*) arasına haritalama
    // Aynı semantik tipe karşılık gelen tek bir LLVM tipi instance'ı olmalı
    std::unordered_map<const Type*, llvm::Type*> semanticToLLVMTypeMap;

    // =======================================================================
    // Semantik Tipleri LLVM Tiplerine Çeviren Yardımcı Metodlar
    // =======================================================================

    // Genel semantik tipi LLVM tipine çevir
    llvm::Type* getLLVMType(Type* type);

    // Temel tipleri LLVM tiplerine çevir
    llvm::Type* getLLVMIntType(const IntType* type);
    llvm::Type* getLLVMFloatType(const FloatType* type); // LLVM'de float genellikle double'a haritalanır
    llvm::Type* getLLVMStringType(const StringType* type); // String temsili (pointer + length?)
    llvm::Type* getLLVMCharType(const CharType* type);
    llvm::Type* getLLVMBoolType(const BoolType* type);
    llvm::Type* getLLVMVoidType(const VoidType* type);

    // Kullanıcı tanımlı ve bileşik tipleri LLVM tiplerine çevir
    llvm::Type* getLLVMStructType(const StructType* type); // LLVM StructType'a haritalanır
    llvm::Type* getLLVMEnumType(const EnumType* type);   // Enum temsili (genellikle int)
    llvm::Type* getLLVMArrayType(const ArrayType* type);
    llvm::Type* getLLVMReferenceType(const ReferenceType* type); // LLVM PointerType'a haritalanır
     llvm::Type* getLLVMPointerType(const PointerType* type); // Eğer *const, *mut varsa
    llvm::Type* getLLVMFunctionType(const FunctionType* type); // LLVM FunctionType'a haritalanır


    // =======================================================================
    // LLVM IR Üretimi Yardımcı Metodları
    // =======================================================================

    // Belirli bir fonksiyonun giriş bloğunda AllocaInst oluştur (Yerel değişkenler için)
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* TheFunction, llvm::Type* VarType, const std::string& VarName);

    // Sembol tablosundaki bir değişken sembolü için LLVM Value* (AllocaInst*) al
    llvm::Value* getLLVMVariable(SymbolInfo* varSymbol);

     // Sembol tablosundaki bir fonksiyon sembolü için LLVM Function* al
    llvm::Function* getLLVMFunction(SymbolInfo* funcSymbol);

    // =======================================================================
    // AST Düğümlerini Gezen ve LLVM IR Üreten Metodlar
    // Bu metodlar AST yapısına göre birbirini özyinelemeli (recursive) çağırır.
    // Expression üreten metodlar llvm::Value* döndürür.
    // Statement ve Declaration üreten metodlar genellikle void döner (IR üretir, değer döndürmez).
    // =======================================================================

    void generateProgram(ProgramAST* program);

    // Bildirim üreticileri
    void generateDeclaration(DeclarationAST* decl); // Türüne göre alt üreticilere dallanır
    llvm::Function* generateFunctionDecl(FunctionDeclAST* funcDecl);
    void generateStructDecl(StructDeclAST* structDecl); // Structlar genellikle sadece tip tanımıdır
    void generateEnumDecl(EnumDeclAST* enumDecl);     // Enumlar sabitler veya tipler üretebilir
    void generateVarDecl(VarDeclAST* varDecl); // Değişken tahsisi ve başlangıç değeri ataması

    // Deyim üreticileri
    void generateStatement(StatementAST* stmt); // Türüne göre alt üreticilere dallanır
    void generateBlockStatement(BlockStatementAST* block);
    void generateImportStatement(ImportStatementAST* importStmt); // Import SEMA'da işlenir, CG'de etkisi minimal
    void generateReturnStatement(ReturnStatementAST* returnStmt);
    void generateBreakStatement(BreakStatementAST* breakStmt);
    void generateContinueStatement(ContinueStatementAST* continueStmt);
    void generateWhileStatement(WhileStatementAST* whileStmt);
    // void generateIfStatement(IfStatementAST* ifStmt);
    // void generateForStatement(ForStatementAST* forStmt);


    // İfade üreticileri - llvm::Value* döndürürler
    llvm::Value* generateExpression(ExpressionAST* expr); // Türüne göre alt üreticilere dallanır
    llvm::Value* generateIntLiteral(IntLiteralAST* literal);
    llvm::Value* generateFloatLiteral(FloatLiteralAST* literal);
    llvm::Value* generateStringLiteral(StringLiteralAST* literal);
    llvm::Value* generateCharLiteral(CharLiteralAST* literal);
    llvm::Value* generateBoolLiteral(BoolLiteralAST* literal);
    llvm::Value* generateIdentifier(IdentifierAST* identifier); // Değişkenin değerini yükle
    llvm::Value* generateBinaryOp(BinaryOpAST* binaryOp);     // İkili operatörlere karşılık gelen IR komutları
    llvm::Value* generateUnaryOp(UnaryOpAST* unaryOp);       // Tekli operatörlere karşılık gelen IR komutları (&, &mut, *)
    llvm::Value* generateAssignment(AssignmentAST* assignment); // Atama komutu (Store)
    llvm::Value* generateCallExpression(CallExpressionAST* call); // Fonksiyon çağrısı komutu (Call)
    llvm::Value* generateMemberAccess(MemberAccessAST* memberAccess); // Üye erişimi (GetElementPtr ve Load/Store)
    llvm::Value* generateIndexAccess(IndexAccessAST* indexAccess); // Index erişimi (GetElementPtr ve Load/Store)
    llvm::Value* generateMatchExpression(MatchExpressionAST* matchExpr); // Match ifadesine karşılık gelen kontrol akışı (Switch, Branch)

    // =======================================================================
    // CNT Özelliklerine Yönelik Kod Üretimi (Sahiplik, Ödünç Alma, Yaşam Süreleri)
    // SEMA kuralları uyguladı, CG ise bu anlama gelen IR'yi üretir.
    // =======================================================================

    // Non-Copy tipinin taşınması: memcpy komutu
    void generateMove(llvm::Value* sourcePtr, llvm::Value* destPtr, Type* movedType);

    // Non-Copy tipinin kopyalanması: Hata olmalı (SEMA yakalar), veya eğer dil copy semantic destekliyorsa özel fonksiyon çağrısı
    // void generateCopy(llvm::Value* sourcePtr, llvm::Value* destPtr, Type* copiedType);

    // Ödünç alma (&, &mut): Zaten UnaryOp generatöründe pointer üretilir.
    // Yaşam Süreleri: IR seviyesinde doğrudan karşılığı yoktur, SEMA'nın garantisidir.
    // Scope sonlarında kaynakların (Non-Copy tiplerin) temizlenmesi (drop/destructor çağrısı)
    // Bu, her kapsam sonuna drop çağrıları ekleyerek yapılır.
    void generateScopeEndCleanup(Scope* scope); // Scope çıkışında çalışacak
    void generateDrop(llvm::Value* valuePtr, Type* valueType); // Belirli bir değer için drop çağrısı

    // =======================================================================


public:
    // Kurucu: Gerekli referansları alır
    LLVMCodeGenerator(Diagnostics& diag, TypeSystem& ts, SymbolTable& st,
                      llvm::LLVMContext& ctx, llvm::TargetMachine& tm);

    // LLVM IR üretme işlemini başlatır
    std::unique_ptr<llvm::Module> generate(ProgramAST* program);
};

#endif // CNT_COMPILER_CODEGEN_H
