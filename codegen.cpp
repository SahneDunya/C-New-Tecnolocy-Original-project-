#include "codegen.h"

#include <iostream> // Debug çıktıları için

// LLVM IR yazdırmak için
#include "llvm/Support/raw_ostream.h"

// LLVM fonksiyon doğrulama için
#include "llvm/IR/Verifier.h"

// Gerekli diğer LLVM komutları (örneğin AllocaInst için)
#include "llvm/IR/IRBuilder.h" // Zaten dahil edildi
#include "llvm/IR/InstrTypes.h" // AllocaInst gibi komut tipleri için

// Semantik sistem implementasyonları (Genellikle ayrı derlenirler, .h dosyalarını include etmek yeterlidir)
 #include "symbol_table.cpp"
 #include "type_system.cpp"
 #include "ownership_checker.cpp" // Ownership logic'in bir kısmı CG'de de gerekebilir
 #include "module_resolver.cpp" // Modül bilgisi CG'de de lazım olabilir

// AST düğüm implementasyonları (Eğer AST metodları varsa)
 #include "expressions.cpp"
 #include "statements.cpp"
 #include "declarations.cpp"
 #include "types.cpp"


// LLVM Kod Üretici Kurucu
LLVMCodeGenerator::LLVMCodeGenerator(Diagnostics& diag, TypeSystem& ts, SymbolTable& st,
                                     llvm::LLVMContext& ctx, llvm::TargetMachine& tm)
    : diagnostics(diag), typeSystem(ts), symbolTable(st), context(ctx), builder(ctx), targetMachine(tm) {
    // LLVM modülünü oluştur
    module = std::make_unique<llvm::Module>("cnt_module", context);
    // Hedef üçlüsünü ve veri düzenini ayarla
    module->setTargetTriple(targetMachine.getTargetTriple().str());
    module->setDataLayout(targetMachine.createDataLayout());

    // Temel LLVM tiplerini semantic tiplere haritala (initial mapping for built-ins)
    // Bu, getLLVMType içinde lazy olarak da yapılabilir.
    semanticToLLVMTypeMap[typeSystem.getIntType()] = llvm::Type::getInt32Ty(context); // Varsayılan int boyutu 32 bit
    semanticToLLVMTypeMap[typeSystem.getFloatType()] = llvm::Type::getDoubleTy(context); // Varsayılan float boyutu double
    semanticToLLVMTypeMap[typeSystem.getBoolType()] = llvm::Type::getInt1Ty(context); // Boolean 1 bit integer
    semanticToLLVMTypeMap[typeSystem.getCharType()] = llvm::Type::getInt8Ty(context); // Char 8 bit integer
    semanticToLLVMTypeMap[typeSystem.getVoidType()] = llvm::Type::getVoidTy(context);
    // String tipi için bir temsil belirleyin (örn: {i8*, i64} - data pointer ve length struct'ı)
     semanticToLLVMTypeMap[typeSystem.getStringType()] = ... ;
     semanticToLLVMTypeMap[typeSystem.getErrorType()] = llvm::Type::getVoidTy(context); // Hata tipinin IR karşılığı yok, void kullanabiliriz

    // Diğer kompleks tipler (struct, array, reference) ilk kullanımda getLLVMType içinde haritalanır.
}

// LLVM IR üretme işlemini başlatır
std::unique_ptr<llvm::Module> LLVMCodeGenerator::generate(ProgramAST* program) {
    // Program AST'sini gezmeye ve IR üretmeye başla
    generateProgram(program);

    // Üretilen LLVM IR modülünü döndür
    return std::move(module);
}

// Genel semantik tipi LLVM tipine çevir
llvm::Type* LLVMCodeGenerator::getLLVMType(Type* type) {
    if (!type || type->id == Type::ERROR_TYPE) {
        // Hata tipi veya null pointer için uygun LLVM tipi döndür (void olabilir)
         diagnostics.reportInternalError(...); // Bu aslında bir SEMA hatası olmalıydı
        return llvm::Type::getVoidTy(context);
    }

    // Eğer tip zaten haritalanmışsa, cache'den döndür
    if (semanticToLLVMTypeMap.count(type)) {
        return semanticToLLVMTypeMap[type];
    }

    // Tip henüz haritalanmamış, türüne göre uygun LLVM tipini oluştur ve kaydet
    llvm::Type* llvmType = nullptr;

    switch (type->id) {
        case Type::INT_TYPE: llvmType = getLLVMIntType(static_cast<IntType*>(type)); break;
        case Type::FLOAT_TYPE: llvmType = getLLVMFloatType(static_cast<FloatType*>(type)); break;
        case Type::BOOL_TYPE: llvmType = getLLVMBoolType(static_cast<BoolType*>(type)); break;
        case Type::STRING_TYPE: llvmType = getLLVMStringType(static_cast<StringType*>(type)); break;
        case Type::CHAR_TYPE: llvmType = getLLVMCharType(static_cast<CharType*>(type)); break;
        case Type::VOID_TYPE: llvmType = getLLVMVoidType(static_cast<VoidType*>(type)); break;
        case Type::STRUCT_TYPE: llvmType = getLLVMStructType(static_cast<StructType*>(type)); break;
        case Type::ENUM_TYPE: llvmType = getLLVMEnumType(static_cast<EnumType*>(type)); break;
        case Type::ARRAY_TYPE: llvmType = getLLVMArrayType(static_cast<ArrayType*>(type)); break;
        case Type::REFERENCE_TYPE: llvmType = getLLVMReferenceType(static_cast<ReferenceType*>(type)); break;
        // case Type::POINTER_TYPE: llvmType = getLLVMPointerType(static_cast<PointerType*>(type)); break;
        case Type::FUNCTION_TYPE: llvmType = getLLVMFunctionType(static_cast<FunctionType*>(type)); break;
        case Type::ERROR_TYPE: return llvm::Type::getVoidTy(context); // Hata tipi
        default:
             diagnostics.reportInternalError("Bilinmeyen semantik tipin LLVM'e çevrimi isteniyor.");
             return llvm::Type::getVoidTy(context);
    }

    // Oluşturulan LLVM tipini haritaya kaydet
    semanticToLLVMTypeMap[type] = llvmType;
    return llvmType;
}

// Temel tiplerin LLVM'e çevrimi (Örnek implementasyonlar)
llvm::Type* LLVMCodeGenerator::getLLVMIntType(const IntType* type) { return llvm::Type::getInt32Ty(context); }
llvm::Type* LLVMCodeGenerator::getLLVMFloatType(const FloatType* type) { return llvm::Type::getDoubleTy(context); }
llvm::Type* LLVMCodeGenerator::getLLVMStringType(const StringType* type) {
    // String temsili: pointer + length (örn: {i8*, i64})
    llvm::Type* i8PtrType = llvm::Type::getInt8PtrTy(context);
    llvm::Type* i64Type = llvm::Type::getInt64Ty(context);
    return llvm::StructType::get(context, {i8PtrType, i64Type});
}
llvm::Type* LLVMCodeGenerator::getLLVMCharType(const CharType* type) { return llvm::Type::getInt8Ty(context); }
llvm::Type* LLVMCodeGenerator::getLLVMBoolType(const BoolType* type) { return llvm::Type::getInt1Ty(context); }
llvm::Type* LLVMCodeGenerator::getLLVMVoidType(const VoidType* type) { return llvm::Type::getVoidTy(context); }


// Kullanıcı tanımlı ve bileşik tiplerin LLVM'e çevrimi (İskelet)
llvm::Type* LLVMCodeGenerator::getLLVMStructType(const StructType* type) {
    // Struct'ın adını kullanarak eksik LLVM StructType'ı al veya oluştur
    llvm::StructType* structTy = module->getTypeByName(type->declaration->name->name);
    if (structTy) return structTy;

    // Eğer struct ilk defa görülüyorsa, eksik (opaque) tip olarak oluştur
    structTy = llvm::StructType::create(context, type->declaration->name->name);
    semanticToLLVMTypeMap[type] = structTy; // Hemen haritala (recursive tipler için)

    // Struct alanlarının LLVM tiplerini al
    std::vector<llvm::Type*> fieldTypes;
    // Semantic analiz aşamasında struct alanlarının tipleri belirlenmiş olmalı
    // type->declaration->fields vectorunu gezerek her alanın semantik tipini alıp getLLVMType ile LLVM tipine çevirin.
     fieldTypes.push_back(getLLVMType(field->semanticType));

    // Struct'ın body'sini tamamla
     structTy->setBody(fieldTypes);

    return structTy;
}

llvm::Type* LLVMCodeGenerator::getLLVMEnumType(const EnumType* type) {
    // Enum temsili: Genellikle integer tipi (varyant sayısına göre uygun genişlikte)
    // Enum değerleri için sabitler oluşturulabilir.
     return llvm::Type::getInt32Ty(context); // Basit bir int temsil
}

llvm::Type* LLVMCodeGenerator::getLLVMArrayType(const ArrayType* type) {
    // Dizi eleman tipinin LLVM karşılığını al
    llvm::Type* elementType = getLLVMType(type->elementType.get());
    // Dizi boyutu (Eğer sabit boyutlu ise ExpressionAST size'dan alınıp constant olmalı)
     llvm::ConstantInt* arraySize = ...;

    // LLVM ArrayType oluştur
     return llvm::ArrayType::get(elementType, arraySize->getZExtValue());
    return llvm::ArrayType::get(elementType, 0); // Şimdilik dinamik/bilinmeyen boyut varsayalım
}

llvm::Type* LLVMCodeGenerator::getLLVMReferenceType(const ReferenceType* type) {
    // Referanslar LLVM'de pointer olarak temsil edilir.
    llvm::Type* referencedType = getLLVMType(type->referencedType.get());
    return llvm::PointerType::get(referencedType, 0); // Address space 0 (varsayılan)
}

llvm::Type* LLVMCodeGenerator::getLLVMFunctionType(const FunctionType* type) {
     // Dönüş tipinin LLVM karşılığını al
     llvm::Type* returnType = getLLVMType(type->returnType); // FunctionType yapınızda dönüş tipi olmalı

     // Parametre tiplerinin LLVM karşılıklarını al
     std::vector<llvm::Type*> paramTypes;
     // type->parameterTypes vectorunu gezerek her parametrenin semantik tipini alıp getLLVMType ile LLVM tipine çevirin.
      paramTypes.push_back(getLLVMType(paramType));

    // LLVM FunctionType oluştur
     return llvm::FunctionType::get(returnType, paramTypes, false); // false: VarArg değil
     return llvm::FunctionType::get(returnType, false); // Şimdilik parametresiz fonksiyon varsayalım
}


// Belirli bir fonksiyonun giriş bloğunda AllocaInst oluştur
llvm::AllocaInst* LLVMCodeGenerator::createEntryBlockAlloca(llvm::Function* TheFunction, llvm::Type* VarType, const std::string& VarName) {
    // Geçici olarak builder'ın insertion point'ini giriş bloğunun başına taşı
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    // Alloca komutunu oluştur
    return TmpB.CreateAlloca(VarType, nullptr, VarName);
}

// Sembol tablosundaki bir değişken sembolü için LLVM Value* (AllocaInst*) al
llvm::Value* LLVMCodeGenerator::getLLVMVariable(SymbolInfo* varSymbol) {
    // Yerel değişken mi?
    if (codegenContext.variableAllocations.count(varSymbol)) {
        return codegenContext.variableAllocations[varSymbol]; // AllocaInst* döndür
    }
    // Global değişken mi? (LLVM Module'de bulunur)
     llvm::GlobalVariable* globalVar = module->getGlobalVariable(varSymbol->name);
     if (globalVar) return globalVar;

    // Bulunamadı (Bu bir SEMA hatası olmalıydı)
    diagnostics.reportInternalError("Kod üretiminde çözümlenmemiş değişken sembolü.");
    return nullptr;
}

// Sembol tablosundaki bir fonksiyon sembolü için LLVM Function* al
llvm::Function* LLVMCodeGenerator::getLLVMFunction(SymbolInfo* funcSymbol) {
    // Daha önce oluşturulmuş mu?
    if (codegenContext.functionMap.count(funcSymbol)) {
        return codegenContext.functionMap[funcSymbol];
    }
    // LLVM modülünde var mı? (Extern fonksiyonlar gibi)
    llvm::Function* func = module->getFunction(funcSymbol->name);
    if (func) {
         // Haritaya ekle ve döndür
         codegenContext.functionMap[funcSymbol] = func;
         return func;
    }

    // Bulunamadı (Bu bir SEMA hatası olmalıydı)
    diagnostics.reportInternalError("Kod üretiminde çözümlenmemiş fonksiyon sembolü.");
    return nullptr;
}


// =======================================================================
// AST Düğümlerini Gezen ve LLVM IR Üreten Metodlar Implementasyonu
// =======================================================================

void LLVMCodeGenerator::generateProgram(ProgramAST* program) {
    // Global kapsam SEMA tarafından girildi.
    // Global değişken initializasyonları ve fonksiyon tanımları burada üretilir.

    // İlk olarak sadece fonksiyon bildirimlerini oluştur (forward declaration gibi)
    // Bu, fonksiyonların birbirini çağırmasını sağlar.
    for (const auto& decl_ptr : program->declarations) {
        if (FunctionDeclAST* funcDecl = dynamic_cast<FunctionDeclAST*>(decl_ptr.get())) {
            // Fonksiyon tipini al
             FunctionType* funcType = static_cast<FunctionType*>(funcDecl->resolvedSemanticType); // SEMA tarafından eklenmiş tip
             llvm::FunctionType* llvmFuncType = getLLVMFunctionType(funcType);

            // LLVM Function nesnesini oluştur
             llvm::Function* TheFunction = llvm::Function::Create(llvmFuncType, llvm::Function::ExternalLinkage, funcDecl->name->name, *module);

            // Parametre isimlerini ayarla
            // ...

            // Sembol tablosundaki sembol ile LLVM Function arasına haritalama yap
            // SymbolInfo* funcSymbol = symbolTable.lookup(funcDecl->name->name).get();
             codegenContext.functionMap[funcSymbol] = TheFunction;
        }
        // Global değişken bildirimleri de burada sadece tanımlanabilir.
         if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(decl_ptr.get())) {
             if (varDecl->isGlobal) {
        //         // Global değişken tanımı
                  llvm::Type* varType = getLLVMType(varDecl->resolvedSemanticType);
                  llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(*module, varType, varDecl->isMutable, llvm::GlobalValue::ExternalLinkage, nullptr, varDecl->name->name);
                 // Initializer'ı daha sonra atayabiliriz veya burada sabitse atayabiliriz.
             }
         }
    }

    // İkinci olarak, fonksiyon gövdelerini ve global değişken başlangıç değerlerini üret
    for (const auto& decl_ptr : program->declarations) {
        generateDeclaration(decl_ptr.get()); // Detaylı üretimi çağır
    }

    // Global kapsamdan SEMA'da çıkıldı. Burada ek bir scope yönetimi gerekmez.
    // Ancak global scope'taki temizlik (drop) işlemleri gerekebilir.
     generateScopeEndCleanup(globalScope); // Eğer böyle bir mekanizma varsa
}

// Bildirim üreticisi (Türüne göre dallanır)
void LLVMCodeGenerator::generateDeclaration(DeclarationAST* decl) {
     if (FunctionDeclAST* funcDecl = dynamic_cast<FunctionDeclAST*>(decl)) {
        generateFunctionDecl(funcDecl);
    } else if (StructDeclAST* structDecl = dynamic_cast<StructDeclAST*>(decl)) {
        generateStructDecl(structDecl); // Genellikle sadece tip tanımıdır
    } else if (EnumDeclAST* enumDecl = dynamic_cast<EnumDeclAST*>(decl)) {
        generateEnumDecl(enumDecl);     // Genellikle sabitler veya tip tanımıdır
    } else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(decl)) {
         if (varDecl->isGlobal) {
             generateVarDecl(varDecl); // Global değişken initializasyonu
         }
    }
    // ImplBlockAST gibi diğer bildirim türleri burada işlenecek
}

// Fonksiyon Bildirimini Üret
llvm::Function* LLVMCodeGenerator::generateFunctionDecl(FunctionDeclAST* funcDecl) {
    // Sembol tablosundaki sembolü bul (SEMA çözmüştü)
     SymbolInfo* funcSymbol = symbolTable.lookup(funcDecl->name->name).get();
     llvm::Function* TheFunction = getLLVMFunction(funcSymbol); // Daha önce oluşturulmuş LLVM Function'ı al

    // Eğer gövdesi yoksa (extern fonksiyon gibi) geri dön
    if (!funcDecl->body) return nullptr; // TheFunction; // extern fonksiyonlar için TheFunction döndürülür

    // Fonksiyon gövdesini oluşturmaya başla
     llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", TheFunction);
     builder.SetInsertPoint(entryBlock); // Komutları buraya ekle

    // currentFunction bağlamını ayarla
     codegenContext.currentFunction = TheFunction;

    // Fonksiyon parametreleri için tahsis yap ve değerlerini sakla
    // ...

    // Fonksiyon gövdesini (blok deyimi) üret
     generateBlockStatement(funcDecl->body.get());

    // Fonksiyonun sonunda bir dönüş komutu yoksa (void fonksiyonlar için gereklidir)
     if (!builder.GetInsertBlock()->getTerminator()) {
         builder.CreateRetVoid();
     }

    // Üretilen fonksiyonu doğrula (hata ayıklama için çok faydalı)
     llvm::verifyFunction(*TheFunction);

    // currentFunction bağlamını sıfırla
     codegenContext.currentFunction = nullptr;

    // return TheFunction;
    return nullptr; // Placeholder
}


// Değişken Bildirimini Üret (Yerel veya Global Başlangıç)
void LLVMCodeGenerator::generateVarDecl(VarDeclAST* varDecl) {
    // SEMA tarafından çözümlenmiş sembol bilgisini al
     SymbolInfo* varSymbol = varDecl->resolvedSymbol; // AST düğümüne eklenmiş olmalı

    // Değişkenin LLVM tipini al
     llvm::Type* varType = getLLVMType(varSymbol->type);

    if (varDecl->isGlobal) {
        // Global değişken başlangıç değeri ataması
        // LLVM GlobalVariable zaten generateProgram'da oluşturulmuş olabilir.
         llvm::GlobalVariable* globalVar = module->getGlobalVariable(varSymbol->name);
         if (varDecl->initializer) {
        //     // Başlangıç değerini üret (genellikle sabit ifade olmalı global için)
              llvm::Constant* initializerVal = static_cast<llvm::Constant*>(generateExpression(varDecl->initializer.get()));
              globalVar->setInitializer(initializerVal);
         } else {
        //     // Başlangıç değeri yoksa varsayılan değeri ata (sıfır)
              globalVar->setInitializer(llvm::Constant::getNullValue(varType));
         }
    } else { // Yerel değişken
        // currentFunction'ın giriş bloğunda AllocaInst oluştur
         llvm::Function* TheFunction = codegenContext.currentFunction;
         llvm::AllocaInst* alloca = createEntryBlockAlloca(TheFunction, varType, varDecl->name->name);

        // Alloca'yı bağlam haritasına kaydet
         codegenContext.variableAllocations[varSymbol] = alloca;

        // Başlangıç değeri (initializer) varsa, kodunu üret ve atama yap (Store)
         if (varDecl->initializer) {
             llvm::Value* initializerVal = generateExpression(varDecl->initializer.get());
        //     // Sahiplik Kuralları: Eğer initializer non-Copy tip ise, Move (memcpy) yap
             // Eğer Copy tip ise, Load/Store yap
              if (initializerType->isCopyType) { // isCopyType semantik tipte olmalı
                  builder.CreateStore(initializerVal, alloca);
              } else {
                   generateMove(initializerVal, alloca, initializerType); // initializerVal bir pointer olmalı
              }
         }
    }

    // Sahiplik Kuralları: Değişkenin ilk durumu OWNED
     OwnershipChecker'ın CG bağlamında değişkenin tahsis edildiğini bilmesi gerekebilir.
     ownershipChecker.recordCodeGenAllocation(varDecl, alloca);
}


// Blok Deyimi Üret
void LLVMCodeGenerator::generateBlockStatement(BlockStatementAST* block) {
    // Yeni bir kod üretimi kapsamı oluştur
     codegenContext.enterScope(); // CodeGenContext'in scope yönetimi varsa

    // Blok içindeki deyimleri üret
    for (const auto& stmt_ptr : block->statements) {
        generateStatement(stmt_ptr.get());
        // Birdeyimin mevcut bloktan çıkış (return, break, continue) içerip içermediğini kontrol etmek
        // ve ulaşılamayan kod üretmekten kaçınmak karmaşık olabilir.
        // Eğer generateStatement bir terminator instruction (return, branch, switch) ürettiyse,
        // builder'ın insertion point'i geçersiz olur. Devam etmeden önce kontrol etmek gerekir.
         if (!builder.GetInsertBlock() || builder.GetInsertBlock()->getTerminator()) {
             break; // Bu bloktan çıkılıyor, sonraki deyimleri üretmeye gerek yok
         }
    }

    // Kapsam sonu temizliği (drop çağrıları)
     generateScopeEndCleanup(currentCodeGenScope); // Kapsam sonundaki drop çağrılarını üret

    // Kod üretimi kapsamından çık
     codegenContext.exitScope();
}

// Return Deyimi Üret
void LLVMCodeGenerator::generateReturnStatement(ReturnStatementAST* returnStmt) {
    // Fonksiyonun dönüş tipini al (SEMA'dan currentFunctionReturnType)
     Type* returnType = codegenContext.currentFunctionReturnType;

    // Dönüş değeri varsa, ifadeyi üret
     if (returnStmt->returnValue) {
         llvm::Value* retVal = generateExpression(returnStmt->returnValue.get());
    //     // Dönüş değeri tipinin fonksiyon dönüş tipine uyumlu olduğunu SEMA garanti etmiş olmalı.
    //     // Gerekiyorsa tip dönüştürme (cast) komutları ekle.

    //     // Sahiplik Kuralları: Eğer dönüş değeri non-Copy tip ise, sahipliği çağırana aktarılmalıdır.
    //     // Bu genellikle return value optimization (RVO) veya hidden sret (struct return) parametresi ile yapılır.
    //     // Basit durumda Load/Ret komutları yeterli olabilir Copy tipler için.
           if (returnType->isCopyType) {
              builder.CreateRet(retVal); // Değeri döndür
          } else {
               generateMove(retVal, sret_parameter_pointer, returnType); // sret_parameter fonksiyon argümanı olarak geçer
               builder.CreateRetVoid(); // void döndür (değer sret parametresine yazıldı)
          }

     } else { // Dönüş değeri yok (void fonksiyon)
         if (returnType->id != Type::VOID_TYPE) {
    //         // SEMA hatası olmalıydı: non-void fonksiyonda dönüş değeri eksik
             diagnostics.reportInternalError("Non-void fonksiyonda dönüş değeri olmadan return.");
         }
         builder.CreateRetVoid(); // void döndür
     }

    // Return komutu bir bloktaki son komut olmalıdır (terminator instruction).
    // Bu noktadan sonra aynı blokta kod üretmemeye dikkat edin.
}

// Break ve Continue Deyimlerini Üret
void LLVMCodeGenerator::generateBreakStatement(BreakStatementAST* breakStmt) {
    // En içteki döngünün break hedef blokunu al
     if (codegenContext.breakTargets.empty()) {
    //      // SEMA hatası olmalıydı: Döngü dışında break
          diagnostics.reportInternalError("Döngü dışında break.");
          return;
     }
     llvm::BasicBlock* breakTarget = codegenContext.breakTargets.back();

    // Mevcut kapsamdaki temizlik (drop) işlemlerini yap
     generateScopeEndCleanup(currentCodeGenScope); // break'ten önce temizlik

    // Break hedef blokuna dallanma komutu oluştur
     builder.CreateBr(breakTarget);

    // Bu noktadan sonra aynı blokta kod üretmemeye dikkat edin (terminator instruction).
     builder.ClearInsertionPoint(); // Veya builder'ı geçersiz yap
}

void LLVMCodeGenerator::generateContinueStatement(ContinueStatementAST* continueStmt) {
     // En içteki döngünün continue hedef blokunu al
     if (codegenContext.continueTargets.empty()) {
          // SEMA hatası olmalıydı: Döngü dışında continue
          diagnostics.reportInternalError("Döngü dışında continue.");
          return;
     }
     llvm::BasicBlock* continueTarget = codegenContext.continueTargets.back();

     // Mevcut kapsamdaki temizlik (drop) işlemlerini yap (break'ten farklı olarak döngü değişkenleri duruyor olabilir)
      generateScopeEndCleanup(currentCodeGenScope); // continue'dan önce temizlik

    // Continue hedef blokuna dallanma komutu oluştur
     builder.CreateBr(continueTarget);

    // Bu noktadan sonra aynı blokta kod üretmemeye dikkat edin.
     builder.ClearInsertionPoint();
}

// While Döngüsü Üret
void LLVMCodeGenerator::generateWhileStatement(WhileStatementAST* whileStmt) {
    // Döngü için 3 Basic Block oluştur: header (koşul), body, end
     llvm::Function* TheFunction = codegenContext.currentFunction;
     llvm::BasicBlock* headerBB = llvm::BasicBlock::Create(context, "while.header", TheFunction);
     llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context, "while.body", TheFunction);
     llvm::BasicBlock* endBB = llvm::BasicBlock::Create(context, "while.end", TheFunction);

    // Mevcut bloktan header bloğuna dallan
     builder.CreateBr(headerBB);

    // Builder'ın insertion point'ini header bloğuna taşı
     builder.SetInsertPoint(headerBB);

    // Koşul ifadesini üret
     llvm::Value* conditionVal = generateExpression(whileStmt->condition.get());
    // SEMA koşulun boolean olduğunu garanti etmiş olmalı.

    // Koşula göre body veya end bloğuna dallan
     builder.CreateCondBr(conditionVal, bodyBB, endBB);

    // Builder'ın insertion point'ini body bloğuna taşı
     builder.SetInsertPoint(bodyBB);

    // Break ve continue hedeflerini CodeGenContext yığınına ekle
     codegenContext.breakTargets.push_back(endBB);
     codegenContext.continueTargets.push_back(headerBB); // Continue header'a döner

    // Döngü gövdesini üret (blok deyimi)
     generateBlockStatement(whileStmt->body.get());

    // Break ve continue hedeflerini yığından çıkar
     codegenContext.breakTargets.pop_back();
     codegenContext.continueTargets.pop_back();

    // Döngü gövdesi terminator ile bitmiyorsa, header bloğuna geri dallan
     if (!builder.GetInsertBlock() || !builder.GetInsertBlock()->getTerminator()) {
          builder.CreateBr(headerBB); // Tekrar koşulu kontrol et
     }

    // Builder'ın insertion point'ini end bloğuna taşı (döngüden çıkış noktası)
     builder.SetInsertPoint(endBB);
}


// İfade üreticisi (Türüne göre dallanır ve llvm::Value* döndürür)
llvm::Value* LLVMCodeGenerator::generateExpression(ExpressionAST* expr) {
    if (!expr) {
         // diagnostics.reportInternalError("Kod üretiminde null ifade düğümü.");
        return nullptr; // Null pointer veya hata için nullptr döndür
    }

    //std::cout << "Generating IR for Expression: " << expr->getNodeType() << std::endl; // Debug çıktısı

    llvm::Value* value = nullptr; // Üretilen LLVM değeri

    if (IntLiteralAST* lit = dynamic_cast<IntLiteralAST*>(expr)) {
        value = generateIntLiteral(lit);
    } else if (FloatLiteralAST* lit = dynamic_cast<FloatLiteralAST*>(expr)) {
        value = generateFloatLiteral(lit);
    } else if (StringLiteralAST* lit = dynamic_cast<StringLiteralAST*>(expr)) {
        value = generateStringLiteral(lit);
    } else if (CharLiteralAST* lit = dynamic_cast<CharLiteralAST*>(expr)) {
        value = generateCharLiteral(lit);
    } else if (BoolLiteralAST* lit = dynamic_cast<BoolLiteralAST*>(expr)) {
        value = generateBoolLiteral(lit);
    } else if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(expr)) {
        value = generateIdentifier(id);
    } else if (BinaryOpAST* binOp = dynamic_cast<BinaryOpAST*>(expr)) {
        value = generateBinaryOp(binOp);
    } else if (UnaryOpAST* unOp = dynamic_cast<UnaryOpAST*>(expr)) {
        value = generateUnaryOp(unOp);
    } else if (AssignmentAST* assign = dynamic_cast<AssignmentAST*>(expr)) {
        value = generateAssignment(assign);
    } else if (CallExpressionAST* call = dynamic_cast<CallExpressionAST*>(expr)) {
        value = generateCallExpression(call);
    } else if (MemberAccessAST* memberAccess = dynamic_cast<MemberAccessAST*>(expr)) {
        value = generateMemberAccess(memberAccess);
    } else if (IndexAccessAST* indexAccess = dynamic_cast<IndexAccessAST*>(expr)) {
        value = generateIndexAccess(indexAccess);
    } else if (MatchExpressionAST* matchExpr = dynamic_cast<MatchExpressionAST*>(expr)) {
         value = generateMatchExpression(matchExpr);
    }
    // ... Diğer ifade türleri

    else {
         diagnostics.reportInternalError("Kod üretiminde bilinmeyen ifade türü.");
        return nullptr;
    }

    // Üretilen LLVM Value'yi AST düğümüne ekleyebiliriz (Annotation)
     expr->generatedValue = value; // Eğer ExpressionAST'te böyle bir üye varsa

    return value; // Üretilen LLVM değerini döndür
}

// Int Literal Üret
llvm::Value* LLVMCodeGenerator::generateIntLiteral(IntLiteralAST* literal) {
    // İnt değerini LLVM ConstantInt'e çevir
    // Boyut SEMA tarafından belirlenmiş olabilir (örn: i32)
     llvm::Type* type = getLLVMType(literal->resolvedSemanticType); // AST düğümüne SEMA tarafından eklenmiş tip
     return llvm::ConstantInt::get(type, literal->value);
    return llvm::ConstantInt::get(context, llvm::APInt(32, literal->value, true)); // Varsayılan 32-bit signed int
}

// Float Literal Üret
llvm::Value* LLVMCodeGenerator::generateFloatLiteral(FloatLiteralAST* literal) {
     // Float değerini LLVM ConstantFP'ye çevir
    // Boyut SEMA tarafından belirlenmiş olabilir (örn: double)
    // llvm::Type* type = getLLVMType(literal->resolvedSemanticType);
    // return llvm::ConstantFP::get(type, literal->value);
    return llvm::ConstantFP::get(context, llvm::APFloat(literal->value)); // Varsayılan double
}

// String Literal Üret
llvm::Value* LLVMCodeGenerator::generateStringLiteral(StringLiteralAST* literal) {
    // String içeriğini LLVM Global String Constant olarak oluştur
    llvm::Constant* stringConstant = llvm::ConstantDataArray::getString(context, literal->value);

    // Global değişken olarak modüle ekle
    auto globalString = new llvm::GlobalVariable(*module, stringConstant->getType(), true, // constant
                                                 llvm::GlobalValue::PrivateLinkage, stringConstant, ".str"); // İsim

    // String temsili struct'ını oluştur ({i8* pointer, i64 length})
    llvm::Type* stringStructType = getLLVMType(typeSystem.getStringType()); // StringType'ın LLVM karşılığı

    // Pointer'ı al (Global değişkenin adresini i8*'a cast et)
    llvm::Constant* zero = llvm::ConstantInt::get(context, llvm::APInt(32, 0));
    std::vector<llvm::Constant*> indices = {zero, zero}; // GEP için indexler (pointer'ın adresini al)
    llvm::Constant* ptr = llvm::ConstantExpr::getGetElementPtr(globalString->getType(), globalString, indices, true); // InBoundsGEP
    llvm::Constant* ptr_casted = llvm::ConstantExpr::getPointerCast(ptr, llvm::Type::getInt8PtrTy(context)); // i8*'a cast

    // Uzunluğu al
    llvm::Constant* length = llvm::ConstantInt::get(context, llvm::APInt(64, literal->value.length()));

    // Struct değerini oluştur
    llvm::Constant* stringStructValue = llvm::ConstantStruct::get(static_cast<llvm::StructType*>(stringStructType), {ptr_casted, length});

    return stringStructValue; // String değerini temsil eden struct'ı döndür
}


// Binary Operatör İfade Üret
llvm::Value* LLVMCodeGenerator::generateBinaryOp(BinaryOpAST* binaryOp) {
    // Sol ve sağ operandların IR'sini üret
    llvm::Value* leftVal = generateExpression(binaryOp->left.get());
    llvm::Value* rightVal = generateExpression(binaryOp->right.get());

    if (!leftVal || !rightVal) return nullptr; // Operand üretiminde hata olduysa

    // Operand tiplerini al (SEMA tarafından belirlenmiş)
     Type* leftType = binaryOp->left->resolvedSemanticType;
     Type* rightType = binaryOp->right->resolvedSemanticType;

    // Operand tiplerine ve operatöre göre uygun LLVM komutunu oluştur
    // SEMA tiplerin uyumlu olduğunu garanti etmiş olmalı.
    // Eğer gerekiyorsa, farklı sayısal tipler arasında (int vs float) Cast komutları ekleyin.

    switch (binaryOp->op) {
        case Token::TOK_PLUS:
             // Eğer tipler float ise FAdd, int ise Add kullan
              if (leftType->id == Type::FLOAT_TYPE) return builder.CreateFAdd(leftVal, rightVal, "addf");
              else return builder.CreateAdd(leftVal, rightVal, "addi");
             return builder.CreateAdd(leftVal, rightVal, "add"); // Basit varsayım: int
        case Token::TOK_MINUS:
              if (leftType->id == Type::FLOAT_TYPE) return builder.CreateFSub(leftVal, rightVal, "subf");
              else return builder.CreateSub(leftVal, rightVal, "subi");
             return builder.CreateSub(leftVal, rightVal, "sub"); // Basit varsayım: int
        case Token::TOK_STAR:
              if (leftType->id == Type::FLOAT_TYPE) return builder.CreateFMul(leftVal, rightVal, "mulf");
              else return builder.CreateMul(leftVal, rightVal, "muli");
             return builder.CreateMul(leftVal, rightVal, "mul"); // Basit varsayım: int
        case Token::TOK_SLASH:
            // Bölme: Integer için SDiv (signed), Float için FDiv
             if (leftType->id == Type::FLOAT_TYPE) return builder.CreateFDiv(leftVal, rightVal, "divf");
             else return builder.CreateSDiv(leftVal, rightVal, "divi"); // Signed Integer Division
            return builder.CreateSDiv(leftVal, rightVal, "div"); // Basit varsayım: int (signed)
        case Token::TOK_PERCENT:
             // Modül: Integer için SRem (signed)
             if (leftType->id == Type::INT_TYPE) return builder.CreateSRem(leftVal, rightVal, "rem");
              else reportInternalError("Modül operatörü sadece tamsayılar için geçerli."); return nullptr;
             return builder.CreateSRem(leftVal, rightVal, "rem"); // Basit varsayım: int (signed)

        case Token::TOK_EQ:
        case Token::TOK_NE:
            // Eşitlik/Eşitsizlik: Integer için ICmp, Float için FCmp
             if (leftType->id == Type::FLOAT_TYPE) {
                  auto predicate = (binaryOp->op == Token::TOK_EQ) ? llvm::FCmpPredicate::FCMP_OEQ : llvm::FCmpPredicate::FCMP_ONE; // Ordered Equal/Not Equal
                  return builder.CreateFCmp(predicate, leftVal, rightVal, "cmpf");
             } else {
                  auto predicate = (binaryOp->op == Token::TOK_EQ) ? llvm::ICmpPredicate::ICMP_EQ : llvm::ICmpPredicate::ICMP_NE; // Equal/Not Equal
                 return builder.CreateICmp(predicate, leftVal, rightVal, "cmpi");
             }
             auto predicate_eq = (binaryOp->op == Token::TOK_EQ) ? llvm::ICmpPredicate::ICMP_EQ : llvm::ICmpPredicate::ICMP_NE;
             return builder.CreateICmp(predicate_eq, leftVal, rightVal, "cmp"); // Basit varsayım: int comparison

        case Token::TOK_LT:
        case Token::TOK_GT:
        case Token::TOK_LE:
        case Token::TOK_GE:
             // Karşılaştırma: Integer için ICmp (SLT, SGT, SLE, SGE signed), Float için FCmp (OLT, OGT, OLE, OGE ordered)
             if (leftType->id == Type::FLOAT_TYPE) {
                  llvm::FCmpPredicate predicate;
                  if (binaryOp->op == Token::TOK_LT) predicate = llvm::FCmpPredicate::FCMP_OLT;
                  else if (binaryOp->op == Token::TOK_GT) predicate = llvm::FCmpPredicate::FCMP_OGT;
                  else if (binaryOp->op == Token::TOK_LE) predicate = llvm::FCmpPredicate::FCMP_OLE;
                  else predicate = llvm::FCmpPredicate::FCMP_OGE;
                  return builder.CreateFCmp(predicate, leftVal, rightVal, "cmpf");
             } else {
                  llvm::ICmpPredicate predicate;
                  if (binaryOp->op == Token::TOK_LT) predicate = llvm::ICmpPredicate::ICMP_SLT; // Signed Less Than
                  else if (binaryOp->op == Token::TOK_GT) predicate = llvm::ICmpPredicate::ICMP_SGT; // Signed Greater Than
                  else if (binaryOp->op == Token::TOK_LE) predicate = llvm::ICmpPredicate::ICMP_SLE; // Signed Less Equal
                  else predicate = llvm::ICmpPredicate::ICMP_SGE; // Signed Greater Equal
                  return builder.CreateICmp(predicate, leftVal, rightVal, "cmpi");
             }
            llvm::ICmpPredicate predicate_cmp; // Basit varsayım: signed int comparison
            if (binaryOp->op == Token::TOK_LT) predicate_cmp = llvm::ICmpPredicate::ICMP_SLT;
            else if (binaryOp->op == Token::TOK_GT) predicate_cmp = llvm::ICmpPredicate::ICMP_SGT;
            else if (binaryOp->op == Token::TOK_LE) predicate_cmp = llvm::ICmpPredicate::ICMP_SLE;
            else predicate_cmp = llvm::ICmpPredicate::ICMP_SGE;
            return builder.CreateICmp(predicate_cmp, leftVal, rightVal, "cmp");

        case Token::TOK_AND: // Mantıksal VE (&&)
            // SEMA tiplerin bool olduğunu garanti etti. LLVM'de 1-bit int.
            // return builder.CreateLogicalAnd(leftVal, rightVal, "andl"); // Veya CreateAnd
             return builder.CreateAnd(leftVal, rightVal, "andl"); // 1-bit int üzerinde bitwise AND mantıksal AND işlevi görür.
        case Token::TOK_OR: // Mantıksal VEYA (||)
             // SEMA tiplerin bool olduğunu garanti etti. LLVM'de 1-bit int.
             return builder.CreateLogicalOr(leftVal, rightVal, "orl"); // Veya CreateOr
            return builder.CreateOr(leftVal, rightVal, "orl"); // 1-bit int üzerinde bitwise OR mantıksal OR işlevi görür.

        // ... Diğer ikili operatörler (+=, -= vb. -> GenerateAssignment'a yönlendirebilir veya burada ele alabilir)

        default:
             diagnostics.reportInternalError("Kod üretiminde bilinmeyen ikili operatör.");
            return nullptr;
    }
}

// Unary Operatör İfade Üret (!, -, &, &mut, *)
llvm::Value* LLVMCodeGenerator::generateUnaryOp(UnaryOpAST* unaryOp) {
    // Operandın IR'sini üret
    llvm::Value* operandVal = generateExpression(unaryOp->operand.get());
    if (!operandVal) return nullptr;

    // Operand tipini al (SEMA tarafından belirlenmiş)
     Type* operandType = unaryOp->operand->resolvedSemanticType;

    switch (unaryOp->op) {
        case Token::TOK_NOT: // Mantıksal DEĞİL (!)
            // Operandın bool olduğunu SEMA garanti etti. 1-bit int üzerinde XOR 1
              if (operandType->id == Type::BOOL_TYPE) return builder.CreateXor(operandVal, llvm::ConstantInt::get(context, llvm::APInt(1, 1)), "notl");
              else { reportInternalError("! operatörü için uyumsuz tip."); return nullptr; }
             return builder.CreateXor(operandVal, builder.getTrue(), "notl"); // 1-bit XOR 1

        case Token::TOK_MINUS: // Negatif (-)
            // Operandın sayısal olduğunu SEMA garanti etti.
             if (operandType->id == Type::FLOAT_TYPE) return builder.CreateFNeg(operandVal, "negf");
             else if (operandType->id == Type::INT_TYPE) return builder.CreateNeg(operandVal, "negi"); // Signed Negation
             else { reportInternalError("- operatörü için uyumsuz tip."); return nullptr; }
             return builder.CreateNeg(operandVal, "negi"); // Basit varsayım: int negation

        case Token::TOK_AND: // & (Immutable Referans)
            // Operandın bir AllocaInst (değişkenin adresi) veya adresi alınabilir başka bir değer olduğunu SEMA garanti etti.
            // generateExpression(unaryOp->operand.get())'dan dönen değer zaten bir pointer olmalı (&expr) veya AllocaInst'e Load yerine AllocaInst'in kendisi dönmeli.
            // Eğer generateExpression her zaman değeri yüklüyorsa, operandın adresini ayrıca almamız gerekir.
             llvm::Value* operandAddress = getAddressOf(unaryOp->operand.get()); // Adresi alan özel bir yardımcı fonksiyon
             return operandAddress;

            // Basit varsayım: generateExpression(operand) eğer bir değişkense AllocaInst'i dönderiyor olabilir.
            return operandVal; // & operand = operand'ın adresi (AllocaInst veya GEP)

        case Token::TOK_MUT: // &mut (Mutable Referans)
            // Aynı & gibi, ama SEMA mutability'yi kontrol etti. LLVM IR aynı pointer type'ı kullanır.
             llvm::Value* operandAddress = getAddressOf(unaryOp->operand.get());
             return operandAddress;
             return operandVal; // &mut operand = operand'ın adresi

        case Token::TOK_STAR: // * (Dereference)
            // Operandın bir pointer veya referans olduğunu SEMA garanti etti.
            // generateExpression(operand) bir pointer/referans (llvm::Value* type PointerType) dönderiyor olmalı.
             return builder.CreateLoad(operandVal, "deref"); // Pointer'ın gösterdiği değeri yükle

            // Basit varsayım: operandVal bir pointer (AllocaInst veya GEP veya başka bir pointer Value)
             return builder.CreateLoad(getLLVMType(static_cast<ReferenceType*>(unaryOp->operand->resolvedSemanticType)->referencedType.get()), operandVal, "load"); // Yükleme komutu

        default:
            diagnostics.reportInternalError("Kod üretiminde bilinmeyen tekli operatör.");
            return nullptr;
    }
}

// Atama İfadesi Üret (=)
llvm::Value* LLVMCodeGenerator::generateAssignment(AssignmentAST* assignment) {
    // Sağ taraf değerini üret
    llvm::Value* rightVal = generateExpression(assignment->right.get());
    if (!rightVal) return nullptr;

    // Sol tarafın adresini üret (Bu bir değişken, struct alanı veya dizi elemanı adresi olmalı)
    // SEMA sol tarafın atanabilir (l-value) olduğunu garanti etti.
    // generateExpression(assignment->left.get()) değeri değil, adresini döndürmeli özel durum olarak.
     llvm::Value* leftAddress = generateAddressOf(assignment->left.get()); // Adresi alan özel yardımcı fonksiyon
     if (!leftAddress) return nullptr;

    // Sol tarafın adresini generateExpression ile almak yerine, sol taraftaki AST düğümüne göre özel işlem yapalım:
    llvm::Value* leftAddress = nullptr;
     if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(assignment->left.get())) {
         SymbolInfo* varSymbol = id->resolvedSymbol; // SEMA tarafından çözülmüş sembol
         leftAddress = getLLVMVariable(varSymbol); // Değişkenin tahsis adresini al
     } else if (MemberAccessAST* memberAccess = dynamic_cast<MemberAccessAST*>(assignment->left.get())) {
         leftAddress = generateMemberAccess(memberAccess); // Üye erişimi adresi (GEP)
     } else if (IndexAccessAST* indexAccess = dynamic_cast<IndexAccessAST*>(assignment->left.get())) {
         leftAddress = generateIndexAccess(indexAccess); // Index erişimi adresi (GEP)
     } else {
    //     // SEMA hatası olmalıydı: Atamanın sol tarafı atanabilir değil.
         diagnostics.reportInternalError("Atanabilir olmayan ifadeye atama kodu üretimi.");
         return nullptr;
     }
     // Yukarıdaki mantık generateExpression içinde adres döndüren bir özel durum olarak da implemente edilebilir.
     // Varsayalım generateExpression(lvalue_expr) adresi döndürüyor:
     leftAddress = generateExpression(assignment->left.get());
     if (!leftAddress || leftAddress->getType()->isPointerTy() == false) {
         diagnostics.reportInternalError("Atamanın sol tarafı adres döndürmüyor.");
         return nullptr;
     }


    // Atama komutunu (Store) oluştur
    // Sahiplik Kuralları: Eğer sağ taraf değeri non-Copy tip ise, Move (memcpy) yap
    // Eğer Copy tip ise, Load/Store yap
     Type* rightType = assignment->right->resolvedSemanticType;
     if (rightType->isCopyType) {
         builder.CreateStore(rightVal, leftAddress);
     } else {
          generateMove(rightVal, leftAddress, rightType); // rightVal bir pointer olmalı
     }

    // Atama ifadesi genellikle atanan değeri döndürür.
    return rightVal; // Atanan değeri döndür
}


// Fonksiyon Çağrısı Üret
llvm::Value* LLVMCodeGenerator::generateCallExpression(CallExpressionAST* call) {
    // Çağrılan fonksiyonun/metodun IR değerini üret
    // Bu genellikle bir Function* veya sanal metot dispatch pointer'ı olur.
     llvm::Value* calleeVal = generateExpression(call->callee.get()); // Callee Identifier veya MemberAccess olabilir

    // Eğer callee bir IdentifierAST ise, sembol tablosundan Function*'ı bul
     llvm::Function* TheFunction = nullptr;
     if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(call->callee.get())) {
         SymbolInfo* funcSymbol = id->resolvedSymbol; // SEMA çözdü
         TheFunction = getLLVMFunction(funcSymbol);
     } else {
    //     // Metot çağrısı veya başka bir çağrı türü (fonksiyon pointerı?)
          diagnostics.reportInternalError("Şu anda sadece düz fonksiyon çağrıları destekleniyor.");
          return nullptr;
     }

     if (!TheFunction) {
    //     // Hata zaten raporlandıysa
         return nullptr;
     }

    // Argüman ifadelerinin IR'lerini üret
     std::vector<llvm::Value*> argValues;
     for (const auto& arg_ptr : call->arguments) {
         llvm::Value* argVal = generateExpression(arg_ptr.get());
         if (!argVal) return nullptr;
         argValues.push_back(argVal);

        // Sahiplik Kuralları: Argümanlar fonskiyon çağrısına Move veya Copy veya Borrow edilir.
        // SEMA bunu kontrol etti. CG burada LLVM karşılığını üretir.
        // Eğer argüman non-Copy tip ise ve move ediliyorsa, değeri yüklemek yerine adresini geçirin veya özel bir move komutu kullanın.
        // FunctionType'ın argümanları hangi geçirme mekanizmasını kullandığını bilmeli (ByValAttr, ByRefAttr vb.)
         if (argSemanticType->isCopyType) { argValues.push_back(argVal); } // Değeri geç
         else { argValues.push_back(getAddressOf(arg_ptr.get())); } // Adresi geç (move/borrow)
     }


    // Fonksiyon çağrısı komutunu (CallInst) oluştur
     llvm::CallInst* callInst = builder.CreateCall(TheFunction, argValues, "calltmp");

    // Dönüş tipini al (SEMA'dan) ve eğer void değilse üretilen değeri döndür
     Type* returnType = call->resolvedSemanticType; // SEMA tarafından eklenmiş dönüş tipi
     if (returnType->id != Type::VOID_TYPE) {
         return callInst;
     } else {
      return nullptr; // Void dönüş tipi
     }
     return nullptr; // Placeholder
}

// Üye Erişimi Üret (obj.field, obj.method())
llvm::Value* LLVMCodeGenerator::generateMemberAccess(MemberAccessAST* memberAccess) {
     // Base ifadesinin IR'sini üret (Bu bir struct'ın veya pointer/referansın değeri olmalı)
     llvm::Value* baseVal = generateExpression(memberAccess->base.get());
     if (!baseVal) return nullptr;

    // Base tipini al (SEMA'dan)
     Type* baseType = memberAccess->base->resolvedSemanticType;

    // Üye ismini al
     const std::string& memberName = memberAccess->member->name;

    // Base tipine göre üyenin indexini bul (StructType içinde alanların sırası tutulur)
     int memberIndex = findFieldIndex(baseType, memberName); // SemanticAnalyzer veya TypeSystem'da bir helper
     if (memberIndex == -1) {
    //     // SEMA hatası olmalıydı: Tanımsız üye
          diagnostics.reportInternalError("Kod üretiminde çözümlenmemiş üye erişimi.");
         return nullptr;
     }

    // Base bir pointer/referans mı yoksa değer mi?
    // LLVM GEP (GetElementPtr) komutu pointer üzerinde çalışır. Eğer base bir değerse, adresini almamız gerekir.
     llvm::Value* basePtr = baseVal; // Eğer baseVal zaten pointer ise
     if (!baseVal->getType()->isPointerTy()) {
    //     // Base değeri stack'te geçici olarak tahsis edip adresini almamız gerekebilir
    //     // Bu karmaşık olabilir, genellikle SEMA'da value category (l-value/r-value) belirlenir
          diagnostics.reportInternalError("Kod üretiminde üye erişimi için atanabilir değer bekleniyor.");
          return nullptr;
     }


    // GEP komutu ile üyenin adresini hesapla
    // Indices vectoru {0, memberIndex} şeklinde olmalı (ilk 0 pointer'ı dereference etmek için)
     std::vector<llvm::Value*> indices = {llvm::ConstantInt::get(context, llvm::APInt(32, 0)), llvm::ConstantInt::get(context, llvm::APInt(32, memberIndex))};
     llvm::Value* memberAddress = builder.CreateInBoundsGEP(basePtr->getType()->getPointerElementType(), basePtr, indices, memberName + "_addr");

    // Eğer üye erişimi ifadenin sonucu olarak kullanılıyorsa (örn: y = obj.x), değeri yükle
    // Eğer atamanın sol tarafındaysa (obj.x = y), adresi döndürülmeli.
    // Parser generateExpression için adres döndüren özel durumları yönetmeli veya generateAddressOf gibi ayrı metodlar olmalı.
    // Eğer bu metod hep adresi döndürüyorsa:
     return memberAddress;

    // Eğer hem Load hem Store için kullanılıyorsa, çağıran generateExpression Load yapar, generateAssignment Store yapar.
    // Bu durumda bu metod adres döndürmelidir.
     if (memberAccess->isAssignmentTarget) return memberAddress; // Eğer AST'de böyle bir flag varsa
     else return builder.CreateLoad(memberAddress, memberName);


    return nullptr; // Placeholder
}

// Index Erişimi Üret (array[index])
llvm::Value* LLVMCodeGenerator::generateIndexAccess(IndexAccessAST* indexAccess) {
     // Base ifadesinin IR'sini üret (Dizi veya pointer olmalı)
     llvm::Value* baseVal = generateExpression(indexAccess->base.get());
     if (!baseVal) return nullptr;

    // Index ifadesinin IR'sini üret (Tamsayı olmalı)
     llvm::Value* indexVal = generateExpression(indexAccess->index.get());
     if (!indexVal) return nullptr;

    // Base tipini al (SEMA'dan)
     Type* baseType = indexAccess->base->resolvedSemanticType;

    // Base bir pointer/referans veya dizi mi?
    // LLVM GEP komutu pointer üzerinde çalışır.
     llvm::Value* basePtr = baseVal; // Eğer baseVal pointer ise
     if (!baseVal->getType()->isPointerTy() && !baseVal->getType()->isArrayTy()) { // Dizi tipi de GEP ile çalışır
          diagnostics.reportInternalError("Kod üretiminde index erişimi için dizi veya pointer bekleniyor.");
         return nullptr;
     }
     If baseVal is an array *value*, not pointer, allocate space and store it, then get address?
     llvm::Value* basePtr;
     if (baseVal->getType()->isArrayTy()) {
    //     // Array değeri ise, stack'te yer tahsis edip değeri kopyala
          llvm::Type* arrayType = baseVal->getType();
          llvm::AllocaInst* arrayAlloca = createEntryBlockAlloca(codegenContext.currentFunction, arrayType, "arraytmp");
          builder.CreateStore(baseVal, arrayAlloca); // Array değerini tahsis edilen yere kopyala
          basePtr = arrayAlloca; // Adres olarak alloca'yı kullan
     } else {
         basePtr = baseVal; // Zaten pointer/referans
     }


    // Index değerinin tamsayı olduğunu kontrol et (SEMA garanti etmeli)
     if (indexVal->getType()->isIntegerTy() == false) {
          diagnostics.reportInternalError("Kod üretiminde dizi indexi tamsayı değil.");
          return nullptr;
     }

    // GEP komutu ile elemanın adresini hesapla
    // Indices vectoru {0, indexVal} şeklinde olmalı (ilk 0 pointer'ı dereference etmek için)
     std::vector<llvm::Value*> indices = {llvm::ConstantInt::get(context, llvm::APInt(32, 0)), indexVal};
     llvm::Value* elementAddress = builder.CreateInBoundsGEP(basePtr->getType()->getPointerElementType(), basePtr, indices, "index_addr");

    // Eğer index erişimi ifadenin sonucu olarak kullanılıyorsa (örn: x = arr[i]), değeri yükle
    // Eğer atamanın sol tarafındaysa (arr[i] = x), adresi döndürülmeli.
    // Parser generateExpression için adres döndüren özel durumları yönetmeli veya generateAddressOf gibi ayrı metodlar olmalı.
    // Eğer bu metod hep adresi döndürüyorsa:
     return elementAddress;

     if (indexAccess->isAssignmentTarget) return elementAddress; // Eğer AST'de böyle bir flag varsa
     else return builder.CreateLoad(elementAddress, "elem");

    return nullptr; // Placeholder
}


// Match İfadesi Üret
llvm::Value* LLVMCodeGenerator::generateMatchExpression(MatchExpressionAST* matchExpr) {
    // Bu, kontrol akışı (Branch, Switch) ve her kol için ayrı Basic Block gerektiren karmaşık bir yapıdır.
    // LLVM Tutorial'larındaki 'if/then/else' ve 'switch' örneklerine bakabilirsiniz.

    // Eşleşmenin yapılacağı ifadeyi üret
     llvm::Value* valueToMatch = generateExpression(matchExpr->value.get());
     if (!valueToMatch) return nullptr;

    // Eşleşen değerin tipini al (SEMA'dan)
     Type* valueType = matchExpr->value->resolvedSemanticType;

    // Match ifadesinden sonraki Basic Block'u oluştur (tüm kollar buraya dallanacak)
     llvm::Function* TheFunction = codegenContext.currentFunction;
     llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "match.end", TheFunction);

    // Match ifadesinin türüne göre (integer, enum, boolean vb.) Switch veya if-else zinciri oluştur
    // Eğer değer bir integer veya enum ise SwitchInstruction kullanışlıdır.
     if (valueType->id == Type::INT_TYPE || valueType->id == Type::ENUM_TYPE) {
         llvm::SwitchInst* switchInst = builder.CreateSwitch(valueToMatch, mergeBB, matchExpr->arms.size()); // Varsayılan hedef mergeBB

    //     // Her match kolunu işle
         for (const auto& arm_ptr : matchExpr->arms) {
    //         // Pattern'ı analiz et ve Switch case'i için sabit değeri (veya range'i) al
    //         // Bu, pattern AST düğümünün türüne bağlıdır (LiteralPattern, EnumVariantPattern vb.)
    //         // Eğer pattern bir literal ise, o literal'in değerini al ve ConstantInt yap.
    //         // Eğer pattern bir enum varyantı ise, o varyantın altında yatan int değerini al.
    //         // Eğer pattern bir değişken bağlama (_ veya isim) ise, o kol her zaman eşleşir veya sadece değeri bağlar (Match kolları sırayla değerlendirilir).
    //         // Eğer pattern bir struct/enum destructuring ise, o zaman Field/Variant değerlerini karşılaştırmak gerekir, bu Switch'ten daha karmaşık bir if-else zinciri veya iç içe matchler gerektirir.

    //         // Basit varsayım: Pattern sadece Literal veya Enum Varyantı ve altına yatan int değeri var.
              llvm::ConstantInt* caseValue = getPatternConstant(arm_ptr->pattern.get(), valueType); // Pattern'dan constant alan yardımcı fonksiyon

    //         // Kolun gövdesi için ayrı bir Basic Block oluştur
              llvm::BasicBlock* armBB = llvm::BasicBlock::Create(context, "match.arm", TheFunction);
              switchInst->addCase(caseValue, armBB); // Switch'e case ekle

    //         // Builder'ı kol bloğuna taşı
              builder.SetInsertPoint(armBB);

    //         // Kolun sonucunu üreten ifadeyi üret
              llvm::Value* armResult = generateExpression(arm_ptr->result.get());
    //         // Kollardan dönen tiplerin uyumlu olduğunu SEMA garanti etti.

    //         // Kolun sonundan merge bloğuna dallan
             builder.CreateBr(mergeBB);

    //         // Match ifadesinin sonucu, phi komutu ile kollardan gelen değerlerden biri olacaktır.
    //         // Eğer match bir değer döndürüyorsa (ExpressionAST ise), mergeBB'de bir PHI düğümü oluşturulmalı.
    //         // PHI düğümü, hangi bloktan gelindiğine göre doğru değeri seçer.
       phiNode->addIncoming(armResult, armBB);
         }

    //     // Switch komutunu üreten builder'ın insertion point'i hala switch bloğundadır.
    //     // Buradan herhangi bir kol üretilmemişse varsayılan hedefe (mergeBB) gidilir.
    //     // Eğer tüm kollar bir değer üretiyorsa ve mergeBB'ye dallanıyorsa, burası tamamdır.

     } else {
    //     // Diğer tipler veya karmaşık patternler için if-else zinciri veya başka yaklaşımlar
          diagnostics.reportInternalError("Şu anda bu tip için match kodu üretimi desteklenmiyor.");
          return nullptr;
     }

    // Builder'ın insertion point'ini merge bloğuna taşı (tüm kolların birleştiği yer)
    // builder.SetInsertPoint(mergeBB);

    // Match ifadesi bir değer döndürüyorsa, mergeBB'de oluşturulan PHI düğümünü döndür.
    // Eğer match bir deyim ise (değer döndürmüyorsa), nullptr döndür.
     return phiNode; // Eğer değer döndürüyorsa
    return nullptr; // Placeholder
}


// Sahiplik, Ödünç Alma, Yaşam Süreleri: IR Üretimi
// Bu, SEMA kurallarını takip eden LLVM komutlarının üretilmesidir.
// Çok Karmaşık ve dilin detaylarına bağlıdır.
// Burada sadece iskelet ve konseptler verilmiştir.

void LLVMCodeGenerator::generateMove(llvm::Value* sourcePtr, llvm::Value* destPtr, Type* movedType) {
    // sourcePtr: Taşınan değerin mevcut adresi (örn: AllocaInst*)
    // destPtr: Taşınacak yerin adresi (örn: Başka bir AllocaInst*)
    // movedType: Taşınan değerin semantik tipi (non-Copy)

    // Tipin boyutunu al (TargetMachine'den)
     uint64_t size = targetMachine.getDataLayout().getTypeAllocSize(getLLVMType(movedType));

    // memcpy komutunu oluştur: sourcePtr'dan destPtr'ye size bayt kopyala
     builder.CreateMemCpy(destPtr, 0, sourcePtr, 0, size); // align = 0 (varsayılan)

    // SEMA bu noktadan sonra sourcePtr'ın geçerli olmadığını garanti etti.
    // IR seviyesinde bunun doğrudan bir karşılığı yoktur, bu SEMA'nın görevidir.
    // Belki debug info eklenebilir?
}

void LLVMCodeGenerator::generateDrop(llvm::Value* valuePtr, Type* valueType) {
    // valuePtr: Temizlenecek değerin adresi (örn: AllocaInst*)
    // valueType: Temizlenecek değerin semantik tipi (drop/destructor gerektiren)

    // Eğer tip drop gerektiriyorsa (SEMA'da belirlenmeli - Drop trait gibi)
     if (valueType->needsDrop) {
        // Tipin drop fonksiyonunu bul (runtime veya statik dispatch)
        // Bu drop fonksiyonu, iç içe alanları/elemanları da temizlemelidir.
         llvm::Function* dropFn = getDropFunction(valueType); // Drop fonksiyonunu bulan yardımcı

        // Drop fonksiyonunu çağır, değeri pointer olarak geçir
         builder.CreateCall(dropFn, {valuePtr});
     }
}

void LLVMCodeGenerator::generateScopeEndCleanup(Scope* scope) {
    // Bir scope (blok veya fonksiyon sonu) biterken çağrılır.
    // Bu scope'ta tanımlanmış ve hala geçerli olan (taşınmamış) tüm değişkenler için drop çağrıları üretilir.

    // scope içindeki değişkenleri gez
     for (const auto& pair : scope->symbols) {
         SymbolInfo* symbol = pair.second.get();
         if (symbol->declarationNode->isVariable && !symbol->isGlobal) { // Yerel değişken ise
             VarDeclAST* varDecl = static_cast<VarDeclAST*>(symbol->declarationNode);
    //         // OwnershipChecker'dan değişkenin o anki durumunu al (taşındı mı?)
              VariableState state = ownershipChecker.getVariableState(varDecl);

    //         // Eğer değişken taşınmadıysa ve tipi drop gerektiriyorsa
              if (state != VariableState::MOVED && state != VariableState::PARTIALLY_MOVED && symbol->type->needsDrop) {
    //             // Değişkenin adresini al
                  llvm::Value* varPtr = getLLVMVariable(symbol);
                 // Drop çağrısını üret
                  generateDrop(varPtr, symbol->type);
              }
         }
     }

     // Drop çağrıları üretildikten sonra, builder'ın insertion point'inin hala geçerli olduğundan emin olun.
     // Scope'tan çıkış bir terminator (Branch, Return) içerebilir.
     // Eğer blok düzgün bitiyorsa (terminator yoksa), generateBlockStatement'ta oraya bir BranchInstruction eklenmelidir.
}
