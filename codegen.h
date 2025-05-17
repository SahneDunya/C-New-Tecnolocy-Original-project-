#ifndef CNT_COMPILER_LLVM_CODEGEN_H
#define CNT_COMPILER_LLVM_CODEGEN_H

#include "diagnostics.h"           // Hata raporlama
#include "type_system.h"           // CNT Type* -> llvm::Type* çevirimi için
#include "symbol_table.h"          // SymbolInfo* -> llvm::Value* (AllocaInst, Function vb.) haritalaması için
#include "ast.h"                   // AST düğüm türleri
#include "expressions.h"           // İfade AST düğümleri
#include "statements.h"            // Deyim AST düğümleri
#include "declarations.h"          // Bildirim AST düğümleri
#include "types.h"                 // Tip AST düğümleri


// LLVM temel başlıkları
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/Value.h" // Tüm LLVM değerlerinin temel sınıfı
#include "llvm/IR/Type.h"  // LLVM tiplerinin temel sınıfı
#include "llvm/IR/DataLayout.h" // Tip boyutları vb. için

#include <string>
#include <vector>
#include <memory>           // std::unique_ptr için
#include <unordered_map>    // Sembol -> LLVM Value haritalaması için
#include <sstream>          // String oluşturma için


namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // LLVM IR Kod Üreticisi
    class LLVMCodeGenerator {
    private:
        Diagnostics& diagnostics;        // Hata raporlama
        TypeSystem& typeSystem;         // Semantik Tip Sistemi referansı
         SymbolTable& symbolTable;     // Sembol tablosu (AST resolvedSymbol kullanır)

        llvm::LLVMContext& llvmContext; // LLVM bağlamı (main tarafından sahip olunan)
        std::unique_ptr<llvm::Module> llvmModule; // Oluşturulan LLVM modülü (CodeGen tarafından sahip olunan)
        std::unique_ptr<llvm::IRBuilder<>> builder; // LLVM IR Builder (CodeGen tarafından sahip olunan)
        const llvm::TargetMachine& targetMachine; // Hedef makine bilgisi
        const llvm::DataLayout* dataLayout; // Veri düzeni bilgisi (tip boyutları için)

        // CNT SymbolInfo'larını karşılık gelen LLVM Value'larına haritala
        // Değişkenler (yerel): SymbolInfo* -> llvm::AllocaInst*
        // Değişkenler (global): SymbolInfo* -> llvm::GlobalVariable*
        // Fonksiyonlar: SymbolInfo* -> llvm::Function*
        std::unordered_map<const SymbolInfo*, llvm::Value*> symbolValueMap;

        // Kontrol akışı takibi (break/continue hedefleri)
        // İç içe geçmiş döngüler için {breakBasicBlock, continueBasicBlock} çiftlerinin yığını
        struct LoopInfo {
            llvm::BasicBlock* breakBlock;
            llvm::BasicBlock* continueBlock;
            const Scope* loopScope; // Döngünün ait olduğu Scope (Drop için)
        };
        std::vector<LoopInfo> loopStack;

        // Mevcut işlenmekte olan LLVM Fonksiyonu
        llvm::Function* currentLLVMFunction = nullptr;


        // =======================================================================
        // Internal Helper Metodlar
        // =======================================================================

        // Bir CNT semantik Type* objesini karşılık gelen LLVM llvm::Type*'a çevirir
        llvm::Type* getLLVMType(Type* type);

        // Bir CNT semantik Type* objesini fonksiyon parametresi için karşılık gelen LLVM llvm::Type*'a çevirir
        // (Çağrı kurallarına göre farklılık gösterebilir)
        llvm::Type* getLLVMParameterType(Type* type);

        // Bir CNT semantik Type* objesini fonksiyon dönüş tipi için karşılık gelen LLVM llvm::Type*'a çevirir
        llvm::Type* getLLVMReturnType(Type* type);

        // Bir CNT SymbolInfo* objesini karşılık gelen LLVM Value*'ına dönüştürür (haritalamadan veya oluşturarak)
        // Değişkenler için ayırma (alloca), globaller için global değişken, fonksiyonlar için fonksiyon pointerı olabilir.
        llvm::Value* getSymbolValue(const SymbolInfo* symbol); // existing

        // Bir kod üretimi hatası raporla
        void reportCodeGenError(const TokenLocation& location, const std::string& message);

        // Bir LLVM tipinin byte boyutunu döndürür
        llvm::Value* getLLVMSizeOf(llvm::Type* type);

        // Bir CNT tipinin byte boyutunu LLVM sabiti olarak döndürür
        llvm::Value* getCNTTypeSize(Type* type);

        // Fonksiyon giriş bloğunda yerel değişken için yığın alanı tahsis eder
        llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* TheFunction, llvm::Type* varType, const std::string& varName);


        // =======================================================================
        // AST Düğümü IR Üretim Metodları
        // Bu metodlar, belirli AST düğüm türleri için LLVM IR üretir.
        // İfade üretim metodları, ifadenin sonucunu temsil eden bir llvm::Value* döndürür.
        // Deyim/Bildirim üretim metodları genellikle void döner veya üretilen bloğu döndürebilir.
        // =======================================================================

        // Programın tamamı için IR üret
        void generateProgram(ProgramAST* program);

        // Bildirimler için IR üret (fonksiyonlar, globaller, tipler)
        llvm::Value* generateDeclaration(DeclarationAST* decl); // Bildirimin kendisini (Function*, GlobalVariable*) döndürür

        // Belirli bildirim türleri için IR üret
        llvm::Function* generateFunctionDecl(FunctionDeclAST* funcDecl); // Üretilen LLVM Function*'ı döndürür
        llvm::GlobalVariable* generateGlobalVariableDeclaration(VarDeclAST* varDecl); // Global değişken için LLVM GlobalVariable*'ı döndürür
        void generateStructDecl(StructDeclAST* structDecl); // LLVM StructType'ı tanımlar (Struct body'si)
        void generateEnumDecl(EnumDeclAST* enumDecl);     // Enum için LLVM Type'ı tanımlar (genellikle int veya struct/union)

        // Deyimler için IR üret
        void generateStatement(StatementAST* stmt);

        // Belirli deyim türleri için IR üret
        void generateBlockStatement(BlockStatementAST* block); // Kapsamı yönetir, IRBuilder noktasını ayarlar, yerel drop'ları yönetir
        void generateExpressionStatement(ExpressionStatementAST* exprStmt); // İfade IR'ı üretir, sonuç değerini atar
        void generateImportStatement(ImportStatementAST* importStmt);     // Importlar SEMA/ModuleResolver'da çözülür. CodeGen, çözülmüş arayüzden harici sembollere (extern fonksiyonlar/globaller) link verir.
        llvm::Value* generateReturnStatement(ReturnStatementAST* returnStmt); // Dönüş IR'ı üretir, kapsam drop'larını yönetir
        llvm::Value* generateBreakStatement(BreakStatementAST* breakStmt);     // Döngü break bloğuna branch üretir, aradaki drop'ları yönetir
        llvm::Value* generateContinueStatement(ContinueStatementAST* continueStmt); // Döngü continue bloğuna branch üretir, aradaki drop'ları yönetir
        void generateWhileStatement(WhileStatementAST* whileStmt);       // Döngü yapısını (bloklar, branşlar) üretir
         void generateIfStatement(IfStatementAST* ifStmt);
         void generateForStatement(ForStatementAST* forStmt);
         void generateLocalVariableDeclaration(VarDeclAST* varDecl); // Yerel değişken bildirimi (Function entry'de alloca)


        // İfadeler için IR üret
        llvm::Value* generateExpression(ExpressionAST* expr); // İfade sonucunu temsil eden llvm::Value*'ı döndürür

        // Belirli ifade türleri için IR üret
        llvm::Value* generateIntLiteral(IntLiteralAST* literal);
        llvm::Value* generateFloatLiteral(FloatLiteralAST* literal);
        llvm::Value* generateStringLiteral(StringLiteralAST* literal); // Global string sabiti için pointer döndürür
        llvm::Value* generateCharLiteral(CharLiteralAST* literal);
        llvm::Value* generateBoolLiteral(BoolLiteralAST* literal);
        llvm::Value* generateIdentifier(IdentifierAST* identifier); // Değişken için pointer veya fonksiyon pointerı döndürür
        llvm::Value* generateBinaryOp(BinaryOpAST* binaryOp);     // İkili işlemin sonucunu döndürür
        llvm::Value* generateUnaryOp(UnaryOpAST* unaryOp);       // Tekli işlemin sonucunu döndürür (&, * özel)
        llvm::Value* generateAssignment(AssignmentAST* assignment); // Atanan değeri döndürür
        llvm::Value* generateCallExpression(CallExpressionAST* call); // Fonksiyon çağrısı sonucunu döndürür
        llvm::Value* generateMemberAccess(MemberAccessAST* memberAccess); // Üye pointerını veya değerini döndürür
        llvm::Value* generateIndexAccess(IndexAccessAST* indexAccess); // Eleman pointerını veya değerini döndürür
        llvm::Value* generateMatchExpression(MatchExpressionAST* matchExpr); // Switch/branches üretir, sonucunu döndürür

        // Değişken referansları (l-values) için IR üret
        // Bellek konumuna pointer döndürür (llvm::Value* pointer tipi).
        llvm::Value* generateLValue(ExpressionAST* expr);


        // =======================================================================
        // Sahiplik ve Ödünç Alma Semantiği IR Üretimi
        // Bu metodlar, sahiplik/ödünç alma/drop semantiğini LLVM IR'ye çevirir.
        // =======================================================================

        // Bir değeri taşımak için IR üretir (non-Copy tipler için memcpy)
        // sourcePtr: Kaynak konumun pointerı (l-value)
        // targetPtr: Hedef konumun pointerı (l-value)
        // valueType: Taşınan değerin semantik tipi
        void generateMove(llvm::Value* sourcePtr, llvm::Value* targetPtr, Type* valueType);

        // Bir değeri kopyalamak için IR üretir (Copy tipler için load + store veya memcpy)
        // sourceValue: Kaynak değer (r-value) veya kaynak konumun pointerı (l-value)
        // targetPtr: Hedef konumun pointerı (l-value)
        // valueType: Kopyalanan değerin semantik tipi
         void generateCopy(llvm::Value* sourceValue, llvm::Value* targetPtr, Type* valueType); // sourceValue r-value veya pointer olabilir

        // Bir değeri Drop etmek için IR üretir (destructor çağrısı)
        // valuePtr: Drop edilecek değerin konumunun pointerı
        // valueType: Drop edilen değerin semantik tipi
        void generateDrop(llvm::Value* valuePtr, Type* valueType);

        // Belirli bir kapsam dışına çıkan değişkenler için Drop IR'ı üretir
        // Bu, generateBlockStatement, generateReturnStatement, generateJump içinde çağrılır.
        void generateDropsForScopeExit(const Scope* scope);


        // =======================================================================
        // Tip Haritalama ve Boyut Bilgisi Yardımcıları
        // =======================================================================
        // CNT Type* -> llvm::Type* haritası (recursive tipler ve verimlilik için)
        std::unordered_map<const Type*, llvm::Type*> cntToLLVMTypeMap;

        // Bir CNT semantik Type* objesini karşılık gelen LLVM llvm::Type*'a çevirir (cache kullanarak)
        llvm::Type* mapCNTTypeToLLVMType(Type* type);

        // Struct ve Enum tipleri için LLVM Layout bilgilerini hesaplar (alan offsetleri vb.)
        // Bu bilgi StructType içinde saklanabilir veya burada hesaplanabilir.
        void computeStructLayout(StructType* structType);
        void computeEnumLayout(EnumType* enumType);


    public:
        // Kurucu
        // LLVMContext ve TargetMachine referanslarını alır (main tarafından yönetilen objeler).
        // Diagnostics, TypeSystem ve SymbolTable referanslarını alır.
        LLVMCodeGenerator(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, llvm::LLVMContext& llvmCtx, const llvm::TargetMachine& tm);

        // ProgramAST'den LLVM Modülü üretir
        // Üretilen modülün sahipliğini çağırana (main) devreder.
        std::unique_ptr<llvm::Module> generate(ProgramAST* program);
    };

} // namespace cnt_compiler

#endif // CNT_COMPILER_LLVM_CODEGEN_H
