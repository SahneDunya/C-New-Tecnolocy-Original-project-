#include "codegen.h"

// Diğer LLVM başlıkları (kullanılacak LLVM komutlarına göre)
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h" // verifyFunction için
#include "llvm/IR/Intrinsics.h" // memcpy gibi intrinsics için
#include "llvm/Support/raw_ostream.h" // debug yazdırma için
#include "llvm/ADT/APInt.h" // Arbitrary precision integers

#include <cassert> // assert için


namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // Kurucu
    LLVMCodeGenerator::LLVMCodeGenerator(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, llvm::LLVMContext& llvmCtx, const llvm::TargetMachine& tm)
        : diagnostics(diag), typeSystem(ts), /* symbolTable(st), */ llvmContext(llvmCtx), targetMachine(tm) {
        // LLVM Modülü ve Builder'ı oluştur
        llvmModule = std::make_unique<llvm::Module>("cnt_module", llvmContext);
        builder = std::make_unique<llvm::IRBuilder<>>(llvmContext);

        // DataLayout'u al (Type boyutlarını hesaplamak için)
        dataLayout = &targetMachine.getDataLayout();

        // Built-in veya önceden tanımlanmış tipler için LLVM karşılıklarını önceden haritala (isteğe bağlı)
        cntToLLVMTypeMap[typeSystem.getIntType()] = builder->getInt32Ty(); // Varsayım: CNT int = i32
        cntToLLVMTypeMap[typeSystem.getFloatType()] = builder->getFloatTy(); // Varsayım: CNT float = float (f32) veya double (f64)
        cntToLLVMTypeMap[typeSystem.getBoolType()] = builder->getInt1Ty(); // Varsayım: CNT bool = i1
        cntToLLVMTypeMap[typeSystem.getCharType()] = builder->getInt8Ty(); // Varsayım: CNT char = i8
        cntToLLVMTypeMap[typeSystem.getVoidType()] = builder->getVoidTy();
        // StringType için karşılık &i8 veya {i8*, i64} gibi bir struct olabilir.
         cntToLLVMTypeMap[typeSystem.getStringType()] = builder->getInt8PtrTy(); // Basit varsayım: &i8*
         cntToLLVMTypeMap[typeSystem.getErrorType()] = builder->getVoidTy(); // Hata tipi genellikle IR'de temsil edilmez
    }

    // ProgramAST'den LLVM Modülü üretir
    std::unique_ptr<llvm::Module> LLVMCodeGenerator::generate(ProgramAST* program) {
        if (!program) {
            reportCodeGenError(TokenLocation(), "Kod üretimi için null ProgramAST.");
            return nullptr;
        }

        // Tüm global bildirimleri gez ve LLVM karşılıklarını oluştur (fonksiyon prototipleri, global değişkenler, struct/enum tipleri)
        // Bu, SEMA'nın 1. geçişine benzer bir rol oynar, ancak bu sefer LLVM objeleri oluşturulur.
        for (const auto& decl_ptr : program->declarations) {
            // generateDeclaration çağrısı burada LLVM fonksiyonları veya global değişkenleri oluşturacak ancak içlerini doldurmayacak.
            // SEMA resolvedSymbol'leri set etti, bu SymbolInfo* objeleri kullanılacak.
             if (const FunctionDeclAST* funcDecl = dynamic_cast<const FunctionDeclAST*>(decl_ptr.get())) {
                 // Fonksiyon prototipini oluştur (LLVM Function objesi)
                 generateFunctionDecl(funcDecl); // Bu, LLVM Function objesini oluşturur ve symbolValueMap'e kaydeder.
             } else if (const VarDeclAST* varDecl = dynamic_cast<const VarDeclAST*>(decl_ptr.get())) {
                 // Global değişken bildirimi
                 // SEMA global/yerel ayrımını yaptı.
                 // Varsayalım VarDeclAST'te isGlobal üyesi var veya resolvedSymbol'e bakarak global olup olmadığı anlaşılıyor (scope depth).
                 // If global:
                  generateGlobalVariableDeclaration(varDecl); // LLVM GlobalVariable objesini oluşturur ve symbolValueMap'e kaydeder.
             } else if (const StructDeclAST* structDecl = dynamic_cast<const StructDeclAST*>(decl_ptr.get())) {
                  // Struct tipi tanımı (LLVM StructType)
                  generateStructDecl(structDecl); // LLVM StructType'ı oluşturur ve cntToLLVMTypeMap'e veya TypeSystem'e kaydeder.
             } else if (const EnumDeclAST* enumDecl = dynamic_cast<const EnumDeclAST*>(decl_ptr.get())) {
                 // Enum tipi tanımı (LLVM Type)
                 generateEnumDecl(enumDecl); // LLVM Type'ı oluşturur ve cntToLLVMTypeMap'e kaydeder.
             }
            // Diğer bildirim türleri...
        }


        // Şimdi fonksiyon gövdelerini doldur (LLVM IR komutlarını üret)
        // Bu, SEMA'nın 2. geçişine benzer.
        for (const auto& decl_ptr : program->declarations) {
             if (const FunctionDeclAST* funcDecl = dynamic_cast<const FunctionDeclAST*>(decl_ptr.get())) {
                 // Fonksiyon gövdesini üret
                 // LLVM Function objesini symbolValueMap'ten al.
                 llvm::Function* llvmFunc = static_cast<llvm::Function*>(getSymbolValue(funcDecl->resolvedSymbol));
                 assert(llvmFunc != nullptr && "LLVM Function not created in first pass.");

                 // Fonksiyon giriş bloğunu ayarla ve builder'ı konumlandır
                 llvm::BasicBlock* entryBlock = &llvmFunc->getEntryBlock();
                 builder->SetInsertPoint(entryBlock);

                 // currentLLVMFunction durumunu ayarla
                 llvm::Function* oldCurrentFunc = currentLLVMFunction;
                 currentLLVMFunction = llvmFunc;

                 // Fonksiyon argümanları için yığın alanı tahsis et ve başlangıç değerlerini yükle (alloca + store)
                 // Parametreler LLVM Function'ın argüman listesinde yer alır.
                 // Yerel değişkenler Function entry blokta tahsis edilir.
                 // SymbolTable veya AST'den yerel değişkenleri alıp generateLocalVariableDeclaration çağrısını yapabilirsiniz.
                 // Örneğin:
                  for (const auto& arg_ptr : funcDecl->arguments) {
                       llvm::Type* argLLVMType = getLLVMType(arg_ptr->type->resolvedSemanticType);
                       llvm::AllocaInst* argAlloca = createEntryBlockAlloca(llvmFunc, argLLVMType, arg_ptr->name->name);
                       symbolValueMap[arg_ptr->resolvedSymbol] = argAlloca; // Argüman sembolünü alloca'ya haritala
                 //      // LLVM fonksiyon argümanından alloca'ya değeri kopyala (store)
                       builder->CreateStore(llvmFunc->arg_begin() + getArgumentIndex(arg_ptr), argAlloca); // Argüman sırasını bulmanız gerekir.
                  }
                  for (const auto& stmt_ptr : funcDecl->body->statements) { // Yerel değişken bildirimlerini bul
                       if (const VarDeclAST* varDecl = dynamic_cast<const VarDeclAST*>(stmt_ptr.get())) {
                            if (!varDecl->isGlobal) { // Sadece yerel değişkenler
                                generateLocalVariableDeclaration(varDecl); // Alloca'yı oluşturur ve symbolValueMap'e kaydeder
                            }
                       }
                  }


                 // Fonksiyon gövdesini (blok deyimi) üret
                 generateBlockStatement(funcDecl->body.get());

                 // Fonksiyon sonundaki branşı veya dönüşü kontrol et (eğer son blok sonlanmadıysa)
                 // builder->GetInsertBlock()->getTerminator() null ise sonlanmamış demektir.
                 if (builder->GetInsertBlock() && !builder->GetInsertBlock()->getTerminator()) {
                     // Void dönüş tipiyse, implicit return void ekle.
                     if (funcDecl->returnType == nullptr || funcDecl->returnType->resolvedSemanticType->isVoidType()) {
                         builder->CreateRetVoid();
                     } else {
                         // Non-void dönüş tipi ama dönüş deyimi yok. Hata veya unreachable ekle.
                         reportCodeGenError(funcDecl->location, "Non-void dönüş tipine sahip fonksiyon 'return' deyimi ile sonlanmıyor.");
                         builder->CreateUnreachable(); // Ulaşılamaz komutu ekle
                     }
                 }

                 // Fonksiyonu doğrula (isteğe bağlı ama hata ayıklama için çok faydalı)
                 llvm::verifyFunction(*llvmFunc);

                 // currentLLVMFunction durumunu geri yükle
                 currentLLVMFunction = oldCurrentFunc;
             }
            // Diğer bildirim türlerinin (global değişken başlangıç değerleri vb.) üretimi burada tamamlanabilir.
        }


        // Modülü doğrula (isteğe bağlı)
        llvm::verifyModule(*llvmModule);

        // Debug için LLVM IR'ı yazdır
         llvmModule->print(llvm::errs(), nullptr); // stderr'e yazdır

        // Oluşturulan modülün sahipliğini çağırana (main) devret
        return std::move(llvmModule);
    }


    // Bir CNT semantik Type* objesini karşılık gelen LLVM llvm::Type*'a çevirir (cache kullanarak)
    llvm::Type* LLVMCodeGenerator::mapCNTTypeToLLVMType(Type* type) {
        if (!type) return builder->getVoidTy(); // Null tip için void veya i8* (belirsiz pointer)

        // Cache'de var mı bak
        auto it = cntToLLVMTypeMap.find(type);
        if (it != cntToLLVMTypeMap.end()) {
            return it->second; // Varsa döndür
        }

        // Yoksa oluştur ve kaydet
        llvm::Type* llvmType = nullptr;

        switch (type->id) {
            case TypeID::Error: llvmType = builder->getVoidTy(); break; // Hata tipi IR'de void olabilir
            case TypeID::Void: llvmType = builder->getVoidTy(); break;
            case TypeID::Bool: llvmType = builder->getInt1Ty(); break; // i1
            case TypeID::Int: llvmType = builder->getInt32Ty(); break; // i32 (varsayım)
            case TypeID::Float: llvmType = builder->getFloatTy(); break; // float (f32) (varsayım)
            case TypeID::Char: llvmType = builder->getInt8Ty(); break; // i8
            case TypeID::String: {
                 // String tipinin LLVM temsili karmaşık olabilir. Örneğin {i8*, i64} struct'ı (pointer + length) veya i8*'a referans.
                 // Basitlik için i8* (char*) yapalım.
                 llvmType = builder->getInt8PtrTy();
                 break;
            }
            case TypeID::Struct: {
                const StructType* structType = static_cast<const StructType*>(type);
                // İsimli struct tipi oluştur (recursive tipler için önce boş oluşturup sonra alanları set etmek gerekir)
                llvm::StructType* llvmStructType = llvm::StructType::create(llvmContext, structType->name);
                // Cache'e boş tipi ekle (recursive tipler için kullanılabilsin)
                cntToLLVMTypeMap[type] = llvmStructType;

                // Alan tiplerini LLVM tipine çevir
                std::vector<llvm::Type*> fieldTypes;
                for (const auto& field : structType->fields) {
                    fieldTypes.push_back(getLLVMType(field.type)); // Özyinelemeli çağrı
                }
                // Struct'ın body'sini ayarla (alan tiplerini ver)
                llvmStructType->setBody(fieldTypes);
                llvmType = llvmStructType;

                // Layout bilgilerini hesapla (computeStructLayout) ve StructType'ın alanlarına offset bilgilerini kaydet (isteğe bağlı, ama GEP için lazım)
                 computeStructLayout(const_cast<StructType*>(structType)); // Non-const metod çağrısı gerektirebilir
                break;
            }
            case TypeID::Enum: {
                 const EnumType* enumType = static_cast<const EnumType*>(type);
                // Enumlar genellikle integer olarak temsil edilir. İlişkili tipler varsa struct/union gerekebilir.
                // Basit C-like enumlar için integer tipi döndürelim.
                // En büyük varyantın discriminant değerine göre uygun integer boyutu belirlenir.
                llvmType = builder->getInt32Ty(); // Varsayım: enumlar i32
                // İlişkili tipler varsa karmaşık temsil
                break;
            }
            case TypeID::Function: {
                const FunctionType* funcType = static_cast<const FunctionType*>(type);
                // Dönüş ve parametre tiplerini LLVM tipine çevir
                llvm::Type* retLLVMType = getLLVMReturnType(funcType->returnType);
                std::vector<llvm::Type*> paramLLVMTypes;
                for (const auto& paramType : funcType->parameterTypes) {
                    paramLLVMTypes.push_back(getLLVMParameterType(paramType));
                }
                // LLVM Fonksiyon tipini oluştur
                llvmType = llvm::FunctionType::get(retLLVMType, paramLLVMTypes, false); // false: vararg değil
                break;
            }
            case TypeID::Reference: {
                const ReferenceType* refType = static_cast<const ReferenceType*>(type);
                // Referanslar LLVM'de pointer olarak temsil edilir.
                llvm::Type* referencedLLVMType = getLLVMType(refType->referencedType);
                llvmType = referencedLLVMType->getPointerTo(); // Gösterdiği tipin pointerı
                break;
            }
            case TypeID::Array: {
                const ArrayType* arrayType = static_cast<const ArrayType*>(type);
                 // Eleman tipini LLVM tipine çevir
                 llvm::Type* elementLLVMType = getLLVMType(arrayType->elementType);
                 // LLVM Array tipini oluştur
                 if (arrayType->size >= 0) { // Sabit boyutlu dizi
                     llvmType = llvm::ArrayType::get(elementLLVMType, arrayType->size);
                 } else {
                     // Dinamik boyutlu dizi veya slice ([T]): pointer + length struct'ı gibi temsil edilebilir.
                     // {i8*, i64} struct'ına pointer gibi.
                     reportCodeGenError(TokenLocation(), "Dinamik boyutlu diziler veya slice tipleri için LLVM temsili implemente edilmedi.");
                     llvmType = builder->getVoidTy(); // Placeholder
                 }
                break;
            }
            // PointerType, TupleType vb. için implementasyon ekleyin.

            default:
                 reportCodeGenError(TokenLocation(), "Bilinmeyen veya desteklenmeyen CNT tip ID'si: " + getTypeIDString(type->id));
                llvmType = builder->getVoidTy(); // Varsayılan olarak void veya hata
                break;
        }

        // Oluşturulan LLVM tipini cache'e ekle ve döndür.
        cntToLLVMTypeMap[type] = llvmType;
        return llvmType;
    }

    // CNT Type* -> llvm::Type* fonksiyon parametresi için
    llvm::Type* LLVMCodeGenerator::getLLVMParameterType(Type* type) {
        // Eğer küçük structlar by value geçiyorsa burada özel işlem gerekebilir.
        // Genellikle getLLVMType ile aynıdır.
        return getLLVMType(type);
    }

    // CNT Type* -> llvm::Type* fonksiyon dönüş tipi için
    llvm::Type* LLVMCodeGenerator::getLLVMReturnType(Type* type) {
        // Eğer küçük structlar by value dönüyorsa burada özel işlem gerekebilir.
        // Genellikle getLLVMType ile aynıdır.
        return getLLVMType(type);
    }

     // Bir kod üretimi hatası raporla
    void LLVMCodeGenerator::reportCodeGenError(const TokenLocation& location, const std::string& message) {
        diagnostics.reportError(location, "Kod Üretimi Hatası: " + message);
    }

     // Bir LLVM tipinin byte boyutunu döndürür
    llvm::Value* LLVMCodeGenerator::getLLVMSizeOf(llvm::Type* type) {
         if (!dataLayout) {
             reportCodeGenError(TokenLocation(), "DataLayout not available for size calculation.");
             return builder->getInt32(0); // Hata durumunda 0
         }
         // Type size in bytes
         unsigned long long typeSize = dataLayout->getTypeAllocSize(type);
         return builder->getInt64(typeSize); // Boyutu i64 olarak döndür (Genel kullanım için)
         // Veya size_t'nin LLVM tipini belirleyip ona göre döndürün.
    }

    // Bir CNT tipinin byte boyutunu LLVM sabiti olarak döndürür
    llvm::Value* LLVMCodeGenerator::getCNTTypeSize(Type* type) {
        if (!type) return builder->getInt64(0);
        llvm::Type* llvmType = getLLVMType(type);
        if (!llvmType) return builder->getInt64(0);
        return getLLVMSizeOf(llvmType);
    }

    // Fonksiyon giriş bloğunda yerel değişken için yığın alanı tahsis eder
    llvm::AllocaInst* LLVMCodeGenerator::createEntryBlockAlloca(llvm::Function* TheFunction, llvm::Type* varType, const std::string& varName) {
        // Entry block'ın ilk talimatından hemen önce builder'ı konumlandır
        llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
        // Alloca talimatını oluştur
        return TmpB.CreateAlloca(varType, nullptr, varName);
    }

    // Bir CNT SymbolInfo* objesini karşılık gelen LLVM Value*'ına dönüştür (haritalamadan veya oluşturarak)
    llvm::Value* LLVMCodeGenerator::getSymbolValue(const SymbolInfo* symbol) {
        if (!symbol) return nullptr; // Geçersiz sembol

        auto it = symbolValueMap.find(symbol);
        if (it != symbolValueMap.end()) {
            return it->second; // Haritada varsa döndür
        }

        // Eğer sembol global bir değişkense (henüz haritalanmadıysa)?
        // Global değişkenler generateGlobalVariableDeclaration'da haritalanmalıydı.
        // Eğer sembol bir fonksiyon ise (henüz haritalanmadıysa)?
        // Fonksiyonlar generateFunctionDecl'de haritalanmalıydı.
        // Bu fonksiyon genellikle yerel değişkenlere erişim için kullanılır,
        // ve yerel değişkenler generateFunctionDecl içinde alloca ile oluşturulup haritalanır.

        // Eğer buraya gelirse, bu bir hata durumu veya haritalanması unutulmuş bir semboldür.
        reportCodeGenError(symbol->location, "Symbol '" + symbol->name + "' LLVM Value haritasında bulunamadı.");
        return nullptr; // Bulunamadı
    }


    // Generate IR for declarations (functions, globals, types)
    llvm::Value* LLVMCodeGenerator::generateDeclaration(DeclarationAST* decl) {
        if (!decl) return nullptr;

        // Bildirim türüne göre ilgili üretim metodunu çağır
        if (FunctionDeclAST* funcDecl = dynamic_cast<FunctionDeclAST*>(decl)) {
             // Fonksiyon bildirimi üretimi (LLVM Function objesi döndürür)
             // generateProgram içinde prototip zaten oluşturuldu. Burada gövde doldurulur.
              generateFunctionDecl(funcDecl); // Bu artık sadece gövde doldurma rolünde kullanılacak.
             // getSymbolValue(funcDecl->resolvedSymbol) -> LLVM Function* döndürür
             return getSymbolValue(funcDecl->resolvedSymbol); // Mevcut LLVM Function objesini döndür
        } else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(decl)) {
             // Değişken bildirimi üretimi (global veya yerel)
             // Global değişkenler generateProgram'da oluşturuldu.
             // Yerel değişkenler createEntryBlockAlloca ile fonksiyon girişinde oluşturulup generateLocalVariableDeclaration içinde haritalanır.
             // Bu metod, çağrıldığı yere göre global/yerel ayrımı yapıp ilgili değeri döndürmelidir.
             // Varsayalım SEMA resolvedSymbol'e isGlobal bilgisi ekliyor veya scope depth'e bakarak anlıyoruz.
              if (varDecl->resolvedSymbol->isGlobal) { // isGlobal SymbolInfo'daysa
                  return getSymbolValue(varDecl->resolvedSymbol); // GlobalVariable* döndür
              } else { // Yerel
             //     // Yerel değişkenin alloca'sı generateFunctionDecl içinde yapıldı ve haritalandı.
                  return getSymbolValue(varDecl->resolvedSymbol); // AllocaInst* döndür
              }
             // Şu anki basit modelde generateVariableDeclaration sadece global için kullanılıyor.
             // Yerel değişkenler generateFunctionDecl içindeki ayrı mantıkla ele alınıyor.
             // Bu metod sadece generateGlobalVariableDeclaration çağırıyorsa:
              return generateGlobalVariableDeclaration(varDecl); // Sadece global üretimi varsayalım
        } else if (StructDeclAST* structDecl = dynamic_cast<StructDeclAST*>(decl)) {
             // Struct tipi tanımı üretimi (LLVM StructType)
             generateStructDecl(structDecl);
             return nullptr; // Tip tanımı bir değer üretmez
        } else if (EnumDeclAST* enumDecl = dynamic_cast<EnumDeclAST*>(decl)) {
            // Enum tipi tanımı üretimi (LLVM Type)
            generateEnumDecl(enumDecl);
            return nullptr; // Tip tanımı bir değer üretmez
        }
        // Diğer bildirim türleri...

        reportCodeGenError(decl->location, "Kod üretimi için bilinmeyen bildirim türü: " + decl->getNodeType());
        return nullptr;
    }

    // Fonksiyon Bildirimi için IR üret (Prototip ve Gövde)
    // LLVM Function objesini oluşturur (prototip) ve içini doldurur (gövde).
    llvm::Function* LLVMCodeGenerator::generateFunctionDecl(FunctionDeclAST* funcDecl) {
        if (!funcDecl || !funcDecl->resolvedSymbol || !funcDecl->resolvedSymbol->type || !funcDecl->resolvedSymbol->type->isFunctionType()) {
            reportCodeGenError(funcDecl->location, "Geçersiz veya çözülmemiş fonksiyon bildirimi.");
            return nullptr;
        }

        // Semantik fonksiyon tipini al
        FunctionType* funcType = static_cast<FunctionType*>(funcDecl->resolvedSymbol->type);

        // LLVM Fonksiyon tipini al (prototype için)
        llvm::FunctionType* llvmFuncType = static_cast<llvm::FunctionType*>(getLLVMType(funcType));

        // LLVM Function objesini oluştur (Modül içinde benzersiz isimle)
        // External linkage (varsayım) - başka modüllerden çağrılabilir.
        // Internal linkage (static gibi) - sadece bu modülden çağrılabilir.
        // Visibility: funcDecl->isPublic bayrağını kullanarak Public (default) veya Hidden/Protected (isteğe bağlı) ayarlayabilirsiniz.
        llvm::Function* llvmFunc = llvm::Function::Create(
            llvmFuncType,
            llvm::GlobalValue::ExternalLinkage, // Linkage Type
            funcDecl->name->name,               // Fonksiyon ismi
            *llvmModule                       // Ait olduğu modül
        );
        // LLVM Function objesini symbolValueMap'e kaydet
        symbolValueMap[funcDecl->resolvedSymbol] = llvmFunc;

        // Eğer sadece prototip gerekiyorsa (gövde implementasyonu yoksa), burada dönebiliriz.
        // Örneğin, harici fonksiyonlar için import edilen arayüzden geliyorsa.
        // SEMA resolvedSymbol'de isExternal gibi bir bayrak set edebilir.
         If (funcDecl->resolvedSymbol->isExternal) return llvmFunc;


        // Gövdeyi üret
        if (funcDecl->body) {
             // Function entry basic block'u oluştur ve builder'ı konumlandır
             llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(llvmContext, "entry", llvmFunc);
             builder->SetInsertPoint(entryBlock);

             // currentLLVMFunction durumunu ayarla
             llvm::Function* oldCurrentFunc = currentLLVMFunction;
             currentLLVMFunction = llvmFunc;

             // Argüman isimlerini LLVM fonksiyon argümanlarına ver (isteğe bağlı, hata ayıklama için)
              auto arg_it = llvmFunc->arg_begin();
              for (const auto& arg_ptr : funcDecl->arguments) {
                   if (arg_it != llvmFunc->arg_end()) {
                       arg_it->setName(arg_ptr->name->name);
                       arg_it++;
                   }
              }

             // Yerel değişkenler için yığın alanı tahsis et
             // Bu, function entry blokta yapılır. createEntryBlockAlloca kullanılır.
             // Tüm yerel değişken bildirimleri AST'de aranmalı veya SEMA'dan SymbolInfo listesi alınmalı.
             // Basitlik için, burada sadece fonksiyon argümanları için alloca oluşturup store yapalım.
             // Yerel değişkenler parseStatement içinde generateLocalVariableDeclaration(varDecl) ile üretilebilir.
             // SEMA'nın SymbolTable'ı kullanarak fonksiyon kapsamındaki tüm yerel değişken SymbolInfo'larını alabilirsiniz.
              for (const auto& local_symbol_pair : symbolTable.getSymbolsInScope(funcDecl->resolvedSymbol->scope)) { // SymbolTable'dan helper
                   const SymbolInfo* local_symbol = local_symbol_pair.second.get();
                   if (local_symbol->isVariable()) {
                       llvm::Type* varLLVMType = getLLVMType(local_symbol->type);
                       llvm::AllocaInst* varAlloca = createEntryBlockAlloca(llvmFunc, varLLVMType, local_symbol->name);
                       symbolValueMap[local_symbol] = varAlloca; // Yerel değişken sembolünü alloca'ya haritala
             //          // Başlangıç değeri varsa generateAssignment/generateCopy/generateMove burada çağrılır (initialize alloca)
                        if (varDecl->initializer) { generateAssignment(varDecl->name, varDecl->initializer, varDecl->location); }
                   }
              }


             // Fonksiyon gövdesini (blok deyimi) üret
             generateBlockStatement(funcDecl->body.get());

             // Son bloğun bir terminator'ı yoksa implicit return/unreachable ekle
             if (builder->GetInsertBlock() && !builder->GetInsertBlock()->getTerminator()) {
                 if (funcType->returnType->isVoidType()) {
                     builder->CreateRetVoid();
                 } else {
                     reportCodeGenError(funcDecl->location, "Non-void fonksiyon '" + funcDecl->name->name + "' return deyimi ile sonlanmıyor.");
                     builder->CreateUnreachable();
                 }
             }

             // Fonksiyonu doğrula
             llvm::verifyFunction(*llvmFunc);

             // currentLLVMFunction durumunu geri yükle
             currentLLVMFunction = oldCurrentFunc;

        } else {
            // Gövdesi olmayan fonksiyon bildirimi (prototip)
            // Bu genellikle harici fonksiyonlar veya forward declarationlar için geçerlidir.
            // LLVM Function objesi (prototip) zaten oluşturuldu.
        }


        return llvmFunc; // Oluşturulan veya bulunan LLVM Function objesini döndür
    }

    // Global Değişken Bildirimi için IR üret
    llvm::GlobalVariable* LLVMCodeGenerator::generateGlobalVariableDeclaration(VarDeclAST* varDecl) {
        if (!varDecl || !varDecl->resolvedSymbol || !varDecl->resolvedSymbol->isVariable()) {
            reportCodeGenError(varDecl->location, "Geçersiz veya çözülmemiş global değişken bildirimi.");
            return nullptr;
        }

        // Global değişkenin LLVM tipini al
        llvm::Type* varLLVMType = getLLVMType(varDecl->resolvedSymbol->type);

        // Başlangıç değeri (varsa)
        llvm::Constant* initializer = nullptr;
        if (varDecl->initializer) {
             // Başlangıç ifadesini üret, sonucunun bir LLVM Constant olması gerekir (Global değişkenler sabit değerlerle başlatılmalı)
             // Bu karmaşık olabilir, compile-time constant ifadesi çözümlemesi gerekir.
             // llvm::Value* initValue = generateExpression(varDecl->initializer.get()); // Bu runtime değer üretebilir!
             // Compile-time constant değer üreten özel bir helper gerekebilir.
             // initializer = getConstantValue(varDecl->initializer.get());
              reportCodeGenError(varDecl->initializer->location, "Global değişken başlangıç değerlerinin compile-time constant olması gerekir. Implementasyon eksik.");
              // Hata durumunda sıfır başlatıcı kullan
              initializer = llvm::Constant::getNullValue(varLLVMType);

        } else {
            // Başlangıç değeri yoksa, varsayılan sıfır değeri kullanılır.
            initializer = llvm::Constant::getNullValue(varLLVMType);
        }

        // Global değişkeni oluştur
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *llvmModule,                    // Ait olduğu modül
            varLLVMType,                    // Değişkenin LLVM tipi
            !varDecl->resolvedSymbol->isMutable, // isConstant (mutable değilse sabit)
            llvm::GlobalValue::ExternalLinkage, // Linkage type (public ise external)
            initializer,                    // Başlangıç değeri
            varDecl->resolvedSymbol->name   // Değişken ismi
        );

        // Global değişkenin LLVM Value'sunu sembol haritasına kaydet
        symbolValueMap[varDecl->resolvedSymbol] = globalVar;

        return globalVar;
    }


    // Struct Tipi Tanımı için IR üret (LLVM StructType)
    void LLVMCodeGenerator::generateStructDecl(StructDeclAST* structDecl) {
        if (!structDecl || !structDecl->resolvedSemanticType || !structDecl->resolvedSemanticType->isStructType()) {
            reportCodeGenError(structDecl->location, "Geçersiz veya çözülmemiş struct bildirimi.");
            return;
        }
        // Struct Type, generateFunctionDecl içinde zaten mapCNTTypeToLLVMType çağrısı ile oluşturulur.
        // Buradaki rol, eğer generateFunctionDecl içinde sadece boş bir tanımlayıcı (opaque) StructType oluşturulduysa,
        // burada setBody metodu ile alanlarını doldurmaktır. Veya TypeSystem, StructType objesini alanlarıyla birlikte
        // tek seferde oluşturuyorsa, bu metoda gerek kalmayabilir veya sadece layout hesaplama için kullanılabilir.

        // Varsayalım mapCNTTypeToLLVMType, StructType'ı alanlarıyla birlikte oluşturuyor.
        // Bu durumda bu metod sadece layout hesaplama ve debug amaçlı kullanılabilir.

        // Struct'ın LLVM Type'ını al
        llvm::Type* llvmStructType = getLLVMType(structDecl->resolvedSemanticType);
        // Layout hesapla (alan offsetleri için)
        if (llvmStructType && llvmStructType->isStructTy()) {
             computeStructLayout(static_cast<StructType*>(structDecl->resolvedSemanticType)); // CNT StructType'a offsetleri kaydet?
        }
    }


    // Deyimler için IR üret
    void LLVMCodeGenerator::generateStatement(StatementAST* stmt) {
        if (!stmt) return;

        // Deyim türüne göre ilgili üretim metodunu çağır
        if (BlockStatementAST* block = dynamic_cast<BlockStatementAST*>(stmt)) {
            generateBlockStatement(block);
        } else if (ExpressionStatementAST* exprStmt = dynamic_cast<ExpressionStatementAST*>(stmt)) {
            generateExpressionStatement(exprStmt);
        } else if (ImportStatementAST* importStmt = dynamic_cast<ImportStatementAST*>(stmt)) {
            generateImportStatement(importStmt);
        } else if (ReturnStatementAST* returnStmt = dynamic_cast<ReturnStatementAST*>(stmt)) {
            generateReturnStatement(returnStmt); // Dönüş bir terminator üretir
        } else if (BreakStatementAST* breakStmt = dynamic_cast<BreakStatementAST*>(stmt)) {
            generateBreakStatement(breakStmt); // Branch üretir
        } else if (ContinueStatementAST* continueStmt = dynamic_cast<ContinueStatementAST*>(stmt)) {
            generateContinueStatement(continueStmt); // Branch üretir
        } else if (WhileStatementAST* whileStmt = dynamic_cast<WhileStatementAST*>(stmt)) {
             generateWhileStatement(whileStmt); // Branşlar ve bloklar üretir
        }
        // IfStatementAST, ForStatementAST gibi diğer deyim türleri
        // VarDeclAST (Yerel değişken bildirimi) generateLocalVariableDeclaration içinde ele alınır,
        // generateStatement burada VarDeclAST görmemeli (veya görürse generateLocalVariableDeclaration çağrısı yapabilir).
         else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
             // Yerel değişken bildirimi, Statement bağlamında
             // Alloca zaten function entry'de yapıldı. Burada sadece başlangıç değerini üretip atama yapıyoruz.
              generateLocalVariableDeclaration(varDecl); // Bu metod alloca ve başlangıç atamasını yapabilir.
             // Alternatif olarak, generateAssignment çağrısı yapın:
             if (varDecl->initializer) {
                  generateAssignment(varDecl->name.get(), varDecl->initializer.get(), varDecl->location); // Atama ifadesi gibi davran
             }
         }

        else {
            // Eğer son üretilen komut bir terminator değilse, bir sonraki bloka düşme olayı olabilir veya bu bir hata.
            // LLVM'de bloklar açık bırakılamaz, bir terminator (ret, br, switch, unreachable) ile sonlanmalıdır.
            // generateBlockStatement içinde bu kontrol yapılmalıdır.
            reportCodeGenError(stmt->location, "Kod üretimi için bilinmeyen deyim türü: " + stmt->getNodeType());
        }
    }

    // Blok Deyimi için IR üret
    void LLVMCodeGenerator::generateBlockStatement(BlockStatementAST* block) {
        if (!block) return;

        // Yeni bir BasicBlock oluşturmak ve builder'ı oraya taşımak isteyebilirsiniz,
        // ancak genellikle blok deyimi içinde yeni bir block açılmaz (unless it's part of if/while/match body).
        // Sadece mevcut builder konumunda deyimleri üretiriz.
        // Kapsam yönetimi (drop'lar için) SEMA'nın Scope bilgisi üzerinden yapılır.

        // Kapsam girişini işaretle (Drop'lar için)
        // OwnershipChecker::enterScope çağrısı SEMA'da yapıldı. CodeGen'in Scope'u bilmesi gerekebilir.
        // LoopInfo'ya scope pointer'ı ekledik. Bloklar için de stack tutulabilir.
         struct BlockScopeInfo { const Scope* scope; };
         std::vector<BlockScopeInfo> blockScopeStack;
         blockScopeStack.push_back({block->associatedScope}); // block->associatedScope SEMA tarafından set edildiyse

        // Blok içindeki deyimleri üret
        for (const auto& stmt_ptr : block->statements) {
            generateStatement(stmt_ptr.get());
            // Eğer generateStatement bir terminator (return, break, continue) ürettiyse,
            // builder'ın insertion point'i null olur veya başka bir bloka geçer.
            // Bu durumda bu bloktaki kalan deyimleri üretmeyi durdurmalıyız.
            if (!builder->GetInsertBlock()) break;
        }

        // Kapsam çıkışındaki drop'ları üret
         generateDropsForScopeExit(block->associatedScope); // block->associatedScope SEMA tarafından set edildiyse

        // Kapsam çıkışını işaretle
         blockScopeStack.pop_back();
    }

    // Return Deyimi için IR üret
    llvm::Value* LLVMCodeGenerator::generateReturnStatement(ReturnStatementAST* returnStmt) {
        if (!returnStmt || !currentLLVMFunction) {
            reportCodeGenError(returnStmt ? returnStmt->location : TokenLocation(), "Geçersiz return deyimi veya mevcut fonksiyon yok.");
            return nullptr;
        }

        // Dönüş değeri varsa, ifadesini üret
        llvm::Value* returnValue = nullptr;
        if (returnStmt->returnValue) {
            returnValue = generateExpression(returnStmt->returnValue.get());
            if (!returnValue) {
                reportCodeGenError(returnStmt->returnValue->location, "Dönüş değeri ifadesi üretilemedi.");
                // Hata durumunda default değer veya undefined kullanabilirsiniz.
                 returnValue = llvm::UndefValue::get(getLLVMType(returnStmt->returnValue->resolvedSemanticType));
            }
             // Eğer dönüş tipi non-Copy ise, bu bir Move'dur. generateMove kullanılabilir.
             // SEMA organize etmeli: return değeri, fonksiyonun dönüş değeri tahsis edildiği yere taşınır.
        } else {
            // Dönüş değeri yoksa (void dönüş tipiyse)
            if (!currentLLVMFunction->getReturnType()->isVoidTy()) {
                 reportCodeGenError(returnStmt->location, "Non-void fonksiyon 'return' deyiminde değer döndürmüyor.");
                 // Hata durumunda undefined değer döndürebilir.
                  returnValue = llvm::UndefValue::get(currentLLVMFunction->getReturnType());
            }
             // returnValue nullptr kalır (void dönüş için).
        }

        // Return komutunu üret
        if (currentLLVMFunction->getReturnType()->isVoidTy()) {
            builder->CreateRetVoid();
        } else {
             if (returnValue) {
                 // Dönüş değeri tipinin fonksiyonun dönüş tipine uyumlu olduğunu SEMA kontrol etti.
                 // Burada sadece değeri döndürüyoruz.
                 builder->CreateRet(returnValue);
             } else {
                // Hata durumunda buraya düşebilir. Unreachable eklemek daha güvenli olabilir.
                builder->CreateUnreachable();
             }
        }

        // Not: Return deyimi, bulunduğu kapsamdan ve tüm üst kapsamlarından fonksiyonun kapsamına kadar olan değişkenlerin
        // drop edilmesine neden olur. generateDropsForScopeExit burada veya generateStatement içinde return görüldüğünde çağrılmalıdır.
         generateDropsForScopeExit(returnStmt->getContainingScope()); // AST düğümünden kapsamı alma helper gerekebilir.

        // Builder'ın insertion point'ini null yap, çünkü bu bir terminator komuttur.
        builder->ClearInsertionPoint();

        return nullptr; // Dönüş deyimi bir değer üretmez
    }

     // Break Deyimi için IR üret
    llvm::Value* LLVMCodeGenerator::generateBreakStatement(BreakStatementAST* breakStmt) {
        if (!breakStmt || loopStack.empty()) {
            reportCodeGenError(breakStmt ? breakStmt->location : TokenLocation(), "'break' deyimi döngü dışında kullanıldı veya loopStack boş.");
            builder->CreateUnreachable(); // Hata durumunda unreachable
            builder->ClearInsertionPoint();
            return nullptr;
        }

        // Döngü yığınından break hedef bloğunu al
        llvm::BasicBlock* breakTargetBlock = loopStack.back().breakBlock;

        // Break deyiminden hedef bloğa kadar olan aradaki kapsamların drop edilmesini üret.
        // jumpStmt'nin ait olduğu kapsam ile break hedef bloğunun ait olduğu kapsam arasındaki değişkenleri bul.
         generateDropsForScopeExit(breakStmt->getContainingScope(), loopStack.back().loopScope); // İki kapsam arası drop helper

        // Break hedef bloğuna branş üret
        builder->CreateBr(breakTargetBlock);

        // Builder'ın insertion point'ini null yap.
        builder->ClearInsertionPoint();

        return nullptr; // Break deyimi bir değer üretmez
    }

     // Continue Deyimi için IR üret
    llvm::Value* LLVMCodeGenerator::generateContinueStatement(ContinueStatementAST* continueStmt) {
        if (!continueStmt || loopStack.empty()) {
            reportCodeGenError(continueStmt ? continueStmt->location : TokenLocation(), "'continue' deyimi döngü dışında kullanıldı veya loopStack boş.");
             builder->CreateUnreachable(); // Hata durumunda unreachable
             builder->ClearInsertionPoint();
            return nullptr;
        }

        // Döngü yığınından continue hedef bloğunu al
        llvm::BasicBlock* continueTargetBlock = loopStack.back().continueBlock;

         // Continue deyiminden hedef bloğa kadar olan aradaki kapsamların drop edilmesini üret.
         generateDropsForScopeExit(continueStmt->getContainingScope(), loopStack.back().loopScope); // İki kapsam arası drop helper

        // Continue hedef bloğuna branş üret
        builder->CreateBr(continueTargetBlock);

        // Builder'ın insertion point'ini null yap.
        builder->ClearInsertionPoint();

        return nullptr; // Continue deyimi bir değer üretmez
    }


    // While Döngüsü için IR üret
    void LLVMCodeGenerator::generateWhileStatement(WhileStatementAST* whileStmt) {
        if (!whileStmt || !currentLLVMFunction) {
             reportCodeGenError(whileStmt ? whileStmt->location : TokenLocation(), "Geçersiz while deyimi veya mevcut fonksiyon yok.");
            return;
        }

        // Mevcut bloktan koşul bloğuna branş (varsa)
        llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(llvmContext, "while.cond", currentLLVMFunction);
        llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(llvmContext, "while.body", currentLLVMFunction);
        llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(llvmContext, "while.end", currentLLVMFunction);

        // Mevcut konumdan koşul bloğuna branş
        if (builder->GetInsertBlock() && !builder->GetInsertBlock()->getTerminator()) {
             builder->CreateBr(conditionBlock);
        } else {
             // Önceki blok bir terminator ürettiyse, builder'ı koşul bloğuna taşı.
             builder->SetInsertPoint(conditionBlock);
        }


        // Koşul bloğuna konumlan
        builder->SetInsertPoint(conditionBlock);

        // Koşul ifadesini üret
        llvm::Value* conditionValue = generateExpression(whileStmt->condition.get());
        if (!conditionValue) {
            reportCodeGenError(whileStmt->condition->location, "While döngüsü koşulu üretilemedi.");
             // Hata durumunda sonsuz döngü veya ulaşılamaz durum ekleyebilirsiniz.
             builder->CreateUnreachable();
             builder->ClearInsertionPoint();
             // Bu blokta kalan deyimleri atla
             return;
        }

        // Koşul değerinin bool (i1) olduğunu SEMA garanti etti.
        // Koşula göre gövde bloğuna veya son blok'a koşullu branş üret
        builder->CreateCondBr(conditionValue, bodyBlock, endBlock);


        // Gövde bloğuna konumlan
        builder->SetInsertPoint(bodyBlock);

        // Döngü yığınına break/continue hedeflerini ekle
        loopStack.push_back({endBlock, conditionBlock, whileStmt->body->associatedScope}); // body->associatedScope SEMA tarafından set edildi

        // Döngü gövdesini üret (blok deyimi)
        generateBlockStatement(whileStmt->body.get()); // generateBlockStatement kendi drop'larını yönetir

        // Döngü gövdesi tamamlandıktan sonra (eğer terminator yoksa), tekrar koşul bloğuna branş üret
        // generateBlockStatement içinde terminator kontrolü yapılmalıdır.
        if (builder->GetInsertBlock() && !builder->GetInsertBlock()->getTerminator()) {
             builder->CreateBr(conditionBlock);
        }

        // Döngü yığınından hedefleri çıkar
        loopStack.pop_back();

        // Döngü sonrası son bloğa konumlan
        builder->SetInsertPoint(endBlock);
    }


    // İfadeler için IR üret (Türüne göre dallanır)
    llvm::Value* LLVMCodeGenerator::generateExpression(ExpressionAST* expr) {
        if (!expr || !currentLLVMFunction) {
             reportCodeGenError(expr ? expr->location : TokenLocation(), "Geçersiz ifade veya mevcut fonksiyon yok.");
            return nullptr;
        }

        // İfade türüne göre ilgili üretim metodunu çağır
        if (IntLiteralAST* lit = dynamic_cast<IntLiteralAST*>(expr)) {
            return generateIntLiteral(lit);
        } else if (FloatLiteralAST* lit = dynamic_cast<FloatLiteralAST*>(expr)) {
            return generateFloatLiteral(lit);
        } else if (StringLiteralAST* lit = dynamic_cast<StringLiteralAST*>(expr)) {
            return generateStringLiteral(lit);
        } else if (CharLiteralAST* lit = dynamic_cast<CharLiteralAST*>(expr)) {
            return generateCharLiteral(lit);
        } else if (BoolLiteralAST* lit = dynamic_cast<BoolLiteralAST*>(expr)) {
            return generateBoolLiteral(lit);
        } else if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(expr)) {
            // Identifier kullanımı, değişkenin kendisi (l-value) veya değeri (r-value) olabilir.
            // Bu metot genellikle r-value bağlamında çağrılır. L-value için generateLValue kullanılır.
            // Burada değeri yüklüyoruz.
            llvm::Value* varPtr = generateLValue(id); // Değişkenin konumunu al
            if (!varPtr) return nullptr;
            // Değeri yükle (load)
            return builder->CreateLoad(getLLVMType(id->resolvedSemanticType), varPtr, id->name.c_str());
        } else if (BinaryOpAST* binOp = dynamic_cast<BinaryOpAST*>(expr)) {
            return generateBinaryOp(binOp);
        } else if (UnaryOpAST* unOp = dynamic_cast<UnaryOpAST*>(expr)) {
            return generateUnaryOp(unOp);
        } else if (AssignmentAST* assign = dynamic_cast<AssignmentAST*>(expr)) {
            // Atama bir değer döndürür (genellikle atanan değer)
            return generateAssignment(assign);
        } else if (CallExpressionAST* call = dynamic_cast<CallExpressionAST*>(expr)) {
            return generateCallExpression(call);
        } else if (MemberAccessAST* memberAccess = dynamic_cast<MemberAccessAST*>(expr)) {
             // Üye erişimi kullanımı, pointer (l-value) veya değer (r-value) olabilir.
             // Burada r-value kullanımı varsayalım (değeri yükle). L-value için generateLValue kullanılır.
             llvm::Value* memberPtr = generateLValue(memberAccess); // Üyenin konumunu al
             if (!memberPtr) return nullptr;
             // Değeri yükle (load)
             return builder->CreateLoad(getLLVMType(memberAccess->resolvedSemanticType), memberPtr, memberAccess->member->name.c_str());

        } else if (IndexAccessAST* indexAccess = dynamic_cast<IndexAccessAST*>(expr)) {
             // Index erişimi kullanımı, pointer (l-value) veya değer (r-value) olabilir.
             // Burada r-value kullanımı varsayalım (değeri yükle). L-value için generateLValue kullanılır.
             llvm::Value* elementPtr = generateLValue(indexAccess); // Elemanın konumunu al
             if (!elementPtr) return nullptr;
             // Değeri yükle (load)
             return builder->CreateLoad(getLLVMType(indexAccess->resolvedSemanticType), elementPtr, "elem");
        } else if (MatchExpressionAST* matchExpr = dynamic_cast<MatchExpressionAST*>(expr)) {
             // Match ifadesi karmaşık kontrol akışı ve değer birleşimi gerektirir.
             return generateMatchExpression(matchExpr);
        }
        // Diğer ifade türleri...

        reportCodeGenError(expr->location, "Kod üretimi için bilinmeyen ifade türü: " + expr->getNodeType());
        return nullptr; // Bilinmeyen ifade için null
    }


    // Generate IR for specific expression types (Implementations)
    llvm::Value* LLVMCodeGenerator::generateIntLiteral(IntLiteralAST* literal) {
        // Semantik tip int olmalı.
        return builder->getInt32(literal->value); // i32 olarak üret
    }

    llvm::Value* LLVMCodeGenerator::generateFloatLiteral(FloatLiteralAST* literal) {
        // Semantik tip float olmalı.
        return llvm::ConstantFP::get(builder->getFloatTy(), literal->value); // f32 olarak üret
    }

    llvm::Value* LLVMCodeGenerator::generateStringLiteral(StringLiteralAST* literal) {
        // String literal LLVM'de global bir sabit dizi (i8*) olarak temsil edilir.
        // builder->CreateGlobalStringPtr, global değişkeni oluşturur ve pointerını döndürür.
        // Name, string içeriğinden türetilebilir veya boş bırakılabilir.
        return builder->CreateGlobalStringPtr(literal->value, "", 0, llvmModule.get()); // String constant üret
    }

    llvm::Value* LLVMCodeGenerator::generateCharLiteral(CharLiteralAST* literal) {
        // Semantik tip char olmalı.
        return builder->getInt8(literal->value); // i8 olarak üret
    }

    llvm::Value* LLVMCodeGenerator::generateBoolLiteral(BoolLiteralAST* literal) {
        // Semantik tip bool olmalı.
        return builder->getInt1(literal->value); // i1 olarak üret
    }

     // Identifier (r-value) generateExpression içinde implemente edildi (generateLValue + CreateLoad)

    // Binary Operatör İfade için IR üret
    llvm::Value* LLVMCodeGenerator::generateBinaryOp(BinaryOpAST* binaryOp) {
        // Sol ve sağ operandları üret
        llvm::Value* leftVal = generateExpression(binaryOp->left.get());
        llvm::Value* rightVal = generateExpression(binaryOp->right.get());

        if (!leftVal || !rightVal) {
             reportCodeGenError(binaryOp->location, "Binary operatör operandları üretilemedi.");
            return nullptr;
        }

        // Operand tipleri SEMA tarafından kontrol edildi, uyumlu oldukları varsayılır.
        llvm::Type* leftType = leftVal->getType();
        llvm::Type* rightType = rightVal->getType();
        // Semantik sonuç tipi: binaryOp->resolvedSemanticType

        // Operatör türüne göre LLVM komutunu seç
        switch (binaryOp->op) {
            case Token::TOK_PLUS:
                if (leftType->isIntegerTy()) return builder->CreateAdd(leftVal, rightVal, "addtmp"); // Tamsayı toplama
                if (leftType->isFloatingPointTy()) return builder->CreateFAdd(leftVal, rightVal, "faddtmp"); // Ondalıklı toplama
                break; // Diğer tipler (string birleştirme vb.) için implementasyon ekleyin.
            case Token::TOK_MINUS:
                if (leftType->isIntegerTy()) return builder->CreateSub(leftVal, rightVal, "subtmp"); // Tamsayı çıkarma
                if (leftType->isFloatingPointTy()) return builder->CreateFSub(leftVal, rightVal, "fsubtmp"); // Ondalıklı çıkarma
                break;
            // ... Diğer aritmetik operatörler (*, /, %) - int için CreateMul, CreateSDiv/UDiv, CreateSRem/URem; float için CreateFMul, CreateFDiv, CreateFRem
            case Token::TOK_EQ:
            case Token::TOK_NE:
            case Token::TOK_LT:
            case Token::TOK_GT:
            case Token::TOK_LE:
            case Token::TOK_GE: {
                 // Karşılaştırma operatörleri
                 llvm::CmpInst::Predicate predicate;
                 if (leftType->isIntegerTy()) {
                     // Tamsayı karşılaştırma (Signed veya Unsigned) - Varsayım Signed
                     switch (binaryOp->op) {
                         case Token::TOK_EQ: predicate = llvm::CmpInst::ICMP_EQ; break; // Eşit
                         case Token::TOK_NE: predicate = llvm::CmpInst::ICMP_NE; break; // Eşit değil
                         case Token::TOK_LT: predicate = llvm::CmpInst::ICMP_SLT; break; // Küçük (Signed)
                         case Token::TOK_GT: predicate = llvm::CmpInst::ICMP_SGT; break; // Büyük (Signed)
                         case Token::TOK_LE: predicate = llvm::CmpInst::ICMP_SLE; break; // Küçük eşit (Signed)
                         case Token::TOK_GE: predicate = llvm::CmpInst::ICMP_SGE; break; // Büyük eşit (Signed)
                         default: return nullptr; // Ulaşılmamalı
                     }
                     return builder->CreateICmp(predicate, leftVal, rightVal, "icmptmp"); // Tamsayı karşılaştırma komutu
                 }
                 if (leftType->isFloatingPointTy()) {
                      // Ondalıklı karşılaştırma (Ordered veya Unordered) - Genellikle Ordered
                      switch (binaryOp->op) {
                          case Token::TOK_EQ: predicate = llvm::CmpInst::FCMP_OEQ; break; // Sıralı Eşit
                          case Token::TOK_NE: predicate = llvm::CmpInst::FCMP_ONE; break; // Sıralı Eşit değil
                          case Token::TOK_LT: predicate = llvm::CmpInst::FCMP_OLT; break; // Sıralı Küçük
                          case Token::TOK_GT: predicate = llvm::CmpInst::FCMP_OGT; break; // Sıralı Büyük
                          case Token::TOK_LE: predicate = llvm::CmpInst::FCMP_OLE; break; // Sıralı Küçük eşit
                          case Token::TOK_GE: predicate = llvm::CmpInst::FCMP_OGE; break; // Sıralı Büyük eşit
                          default: return nullptr; // Ulaşılmamalı
                      }
                      return builder->CreateFCmp(predicate, leftVal, rightVal, "fcmptmp"); // Ondalıklı karşılaştırma komutu
                 }
                 break; // Diğer tipler (string karşılaştırma vb.) için implementasyon ekleyin.
            }
            case Token::TOK_AND: // Mantıksal VE (&&)
                // Sadece bool (i1) için geçerli. Short-circuiting üretilmeli.
                reportCodeGenError(binaryOp->location, "Mantıksal VE (&&) için kısa devre (short-circuiting) üretimi implemente edilmedi.");
                 // Basit (short-circuiting olmayan) boolean AND:
                 if (leftType->isIntegerTy(1) && rightType->isIntegerTy(1)) { // i1 tiplerini kontrol et
                     return builder->CreateAnd(leftVal, rightVal, "andtmp");
                 }
                break;
            case Token::TOK_OR: // Mantıksal VEYA (||)
                // Sadece bool (i1) için geçerli. Short-circuiting üretilmeli.
                reportCodeGenError(binaryOp->location, "Mantıksal VEYA (||) için kısa devre (short-circuiting) üretimi implemente edilmedi.");
                 // Basit (short-circuiting olmayan) boolean OR:
                 if (leftType->isIntegerTy(1) && rightType->isIntegerTy(1)) {
                     return builder->CreateOr(leftVal, rightVal, "ortmp");
                 }
                break;

            default:
                 reportCodeGenError(binaryOp->location, "Kod üretimi için bilinmeyen ikili operatör.");
                break;
        }

        // Hata veya desteklenmeyen operatör/tip kombinasyonu
        return nullptr;
    }

    // Tekli Operatör İfade için IR üret
    llvm::Value* LLVMCodeGenerator::generateUnaryOp(UnaryOpAST* unaryOp) {
         if (!unaryOp) return nullptr;

        // Operandı üret
        llvm::Value* operandVal = generateExpression(unaryOp->operand.get());
        if (!operandVal) {
             reportCodeGenError(unaryOp->location, "Tekli operatör operandı üretilemedi.");
            return nullptr;
        }

        llvm::Type* operandType = operandVal->getType();
        // Semantik sonuç tipi: unaryOp->resolvedSemanticType

        // Operatör türüne göre LLVM komutunu seç
        switch (unaryOp->op) {
            case Token::TOK_MINUS: // Negatif (-)
                 if (operandType->isIntegerTy()) return builder->CreateNeg(operandVal, "negtmp"); // Tamsayı negatif
                 if (operandType->isFloatingPointTy()) return builder->CreateFNeg(operandVal, "fnegtmp"); // Ondalıklı negatif
                 break;
            case Token::TOK_NOT: // Mantıksal DEĞİL (!)
                 if (operandType->isIntegerTy(1)) return builder->CreateNot(operandVal, "nottmp"); // i1 (bool) DEĞİL
                 break;
            case Token::TOK_AND: // & (Immutable Referans)
                 // SEMA, operandın l-value olduğunu kontrol etti. generateLValue operandın adresini verir.
                 // & operatörü, l-value'nun adresini döndürür. generateLValue yeterlidir.
                 return generateLValue(unaryOp->operand.get());

            case Token::TOK_MUT: // &mut (Mutable Referans)
                 // SEMA, operandın l-value ve mutable olduğunu kontrol etti. generateLValue operandın adresini verir.
                 // &mut operatörü de l-value'nun adresini döndürür. generateLValue yeterlidir.
                 return generateLValue(unaryOp->operand.get());

            case Token::TOK_STAR: // * (Dereference)
                 // Operandın bir pointer tipi olduğunu SEMA garanti etti. operandVal bir pointer.
                 // Dereference, pointerın işaret ettiği adresteki değeri yükler (load).
                 if (operandType->isPointerTy()) {
                     // Pointer'ın gösterdiği tip: operandType->getPointerElementType()
                     llvm::Type* pointeeLLVMType = operandType->getPointerElementType();
                     return builder->CreateLoad(pointeeLLVMType, operandVal, "loadtmp"); // Adresteki değeri yükle
                 }
                 break;

            default:
                 reportCodeGenError(unaryOp->location, "Kod üretimi için bilinmeyen tekli operatör.");
                break;
        }

        return nullptr; // Hata veya desteklenmeyen operatör/tip
    }

    // Atama İfadesi için IR üret (=)
    llvm::Value* LLVMCodeGenerator::generateAssignment(AssignmentAST* assignment) {
         if (!assignment || !currentLLVMFunction) return nullptr;

        // Sağ taraf ifadeyi üret
        llvm::Value* rightVal = generateExpression(assignment->right.get());
        if (!rightVal) {
             reportCodeGenError(assignment->location, "Atama sağ tarafı üretilemedi.");
            return nullptr;
        }

        // Sol tarafı l-value olarak üret (adresini al)
        llvm::Value* leftPtr = generateLValue(assignment->left.get());
        if (!leftPtr) {
            reportCodeGenError(assignment->location, "Atama sol tarafı (l-value) üretilemedi.");
            return nullptr;
        }

        // SEMA, atanabilirliği kontrol etti.
        Type* valueType = assignment->right->resolvedSemanticType; // Atanan değerin semantik tipi
        // Hedef tip: assignment->left->resolvedSemanticType (l-value'nun gösterdiği tip olmalı)
        // Target mutable mı? SEMA check etti.

        // Sahiplik/Ödünç alma kurallarına göre taşıma (move) veya kopyalama (copy) IR'ı üret
        if (typeSystem.implementsCopy(valueType)) {
            // Tip Copy ise, kopyalama IR'ı üret (load + store veya memcpy)
             generateCopy(rightVal, leftPtr, valueType); // rightVal r-value
        } else {
            // Tip Non-Copy ise, taşıma IR'ı üret (memcpy)
             // Eğer sağ taraf bir l-value ise, adresini almalıyız.
             // Eğer sağ taraf bir r-value ise, generateExpression sonucu değeri verir, adresini değil.
             // CodeGen'de Move semantics yönetimi, generateExpression'ın non-Copy r-value'lar için
             // pointer döndürmesini veya generateMove/generateCopy metodlarının hem r-value hem l-value kaynağı kabul etmesini gerektirebilir.
             // Basitlik için, generateMove'un adres (pointer) aldığını varsayalım.
             // Eğer rightVal bir değer ise, onu stack'te geçici bir yere store edip adresini almalıyız.
             if (rightVal->getType()->isPointerTy()) { // Sağ taraf zaten bir pointer (l-value)
                 generateMove(rightVal, leftPtr, valueType);
             } else { // Sağ taraf bir değer (r-value)
                 // Geçici alloca oluştur, değeri oraya store et, adresini generateMove'a ver.
                 llvm::Type* valueLLVMType = rightVal->getType();
                 llvm::AllocaInst* tempAlloca = builder->CreateAlloca(valueLLVMType, nullptr, "tmp.move.src");
                 builder->CreateStore(rightVal, tempAlloca);
                 generateMove(tempAlloca, leftPtr, valueType);
             }

        }

        // Atama ifadesi genellikle atanan değeri (sağ tarafın değerini) döndürür.
        return rightVal;
    }

     // Fonksiyon Çağrısı için IR üret
    llvm::Value* LLVMCodeGenerator::generateCallExpression(CallExpressionAST* call) {
         if (!call || !currentLLVMFunction) return nullptr;

        // Çağrılan ifadeyi üret (Genellikle Function* veya fonksiyon pointerı)
        llvm::Value* calleeVal = generateExpression(call->callee.get());
        if (!calleeVal) {
            reportCodeGenError(call->callee->location, "Fonksiyon çağrısı hedefi üretilemedi.");
            return nullptr;
        }

        // Callee tipinin FunctionType* veya PointerToFunctionType olduğunu SEMA garanti etti.
         llvm::FunctionType* llvmFuncType = ... // CalleeVal'in tipinden LLVM FunctionType'ı çıkarın

        // Argüman ifadelerini üret
        std::vector<llvm::Value*> argVals;
        for (const auto& arg_ptr : call->arguments) {
            llvm::Value* argVal = generateExpression(arg_ptr.get());
            if (!argVal) {
                 reportCodeGenError(arg_ptr->location, "Fonksiyon çağrısı argümanı üretilemedi.");
                 return nullptr; // Argüman üretilemezse çağrıyı üretemeyiz.
            }
            // Argüman geçiş kurallarına göre değeri ayarlayın (value, pointer, reference)
            // Örneğin, eğer parametre tipi &T ise ve argVal bir değişkense (AllocaInst*), doğrudan argVal'ı (pointerı) geçin.
            // Eğer parametre tipi T ise ve argVal bir değer ise, onu direkt geçin.
            // Eğer parametre tipi T ise ve argVal bir l-value (pointer) ise, değeri yükleyip geçin.
            // Bu, FunctionType'ın parametrelerinin nasıl geçtiği bilgisine ve argüman ifadesinin l-value/r-value olmasına bağlıdır.
             argVals.push_back(argVal); // Basitçe değerleri geçelim şimdilik.
        }

        // LLVM Çağrı komutunu üret
        // Eğer calleeVal bir Function* ise CreateCall(Function*, Args)
        // Eğer calleeVal bir Function Pointer ise CreateCall(FunctionType*, Pointer, Args)
        // CalleeVal'in hangi türde olduğuna bakmak gerekir. SEMA resolvedCalleeSymbol'de Function* tutuyorsa o kullanılır.
        llvm::Value* callResult = nullptr;
        if (llvm::Function* directCallFunc = dynamic_cast<llvm::Function*>(calleeVal)) {
            callResult = builder->CreateCall(directCallFunc, argVals, "calltmp");
        } else if (calleeVal->getType()->isPointerTy() && calleeVal->getType()->getPointerElementType()->isFunctionTy()) {
             // Fonksiyon pointerı üzerinden çağrı
             llvm::FunctionType* funcPtrType = static_cast<llvm::FunctionType*>(calleeVal->getType()->getPointerElementType());
             callResult = builder->CreateCall(funcPtrType, calleeVal, argVals, "calltmp");
        } else {
            reportCodeGenError(call->callee->location, "Geçersiz fonksiyon çağrı hedefi LLVM değeri.");
            // Hata durumunda bir undef değeri veya sıfır değeri döndür.
             callResult = llvm::UndefValue::get(getLLVMType(call->resolvedSemanticType));
            return nullptr;
        }


        // Sahiplik Kuralları: Argümanların taşınması/ödünç alınması, dönüş değerinin sahipliği.
         handleFunctionCall(call); // OwnershipChecker'a bilgi

        // Çağrı sonucunu döndür (Fonksiyonun dönüş değeri)
        // Eğer dönüş tipi void değilse bir değer döner.
        // SEMA resolvedSemanticType'ı ayarladı.
        return callResult;
    }


    // Generate IR for different kinds of variable references (l-values)
    // Bellek konumuna pointer döndürür.
    llvm::Value* LLVMCodeGenerator::generateLValue(ExpressionAST* expr) {
         if (!expr || !currentLLVMFunction) return nullptr;

        // İfade türüne göre l-value üretimini çağır
        if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(expr)) {
            // Identifier'ın bir değişkene (local veya global) işaret ettiğini SEMA garanti etti.
            // SymbolInfo'sundan karşılık gelen LLVM Value'sunu (AllocaInst* veya GlobalVariable*) al.
            llvm::Value* symbolVal = getSymbolValue(id->resolvedSymbol);
            if (!symbolVal) {
                 reportCodeGenError(id->location, "'" + id->name + "' sembolü için LLVM Value (l-value) bulunamadı.");
                 return nullptr;
            }
            // Sembolün LLVM Value'su zaten bir pointer olmalı (AllocaInst, GlobalVariable).
            assert(symbolVal->getType()->isPointerTy() && "Symbol value is not a pointer type.");
            return symbolVal;

        } else if (MemberAccessAST* memberAccess = dynamic_cast<MemberAccessAST*>(expr)) {
             // Üye erişiminin sol tarafı (l-value)
             // Base ifadesini l-value olarak üret (pointerını al) veya pointer tipi bir ifade ise değerini al.
             llvm::Value* basePtrOrValue = generateLValue(memberAccess->base.get()); // Base'in adresini almayı dene (Struct* gibi)
             // Eğer base ExpressionAST'in resolvedSemanticType'ı ReferenceType (&Struct) ise,
             // generateLValue çağrısı &Struct'ı yükleyip Struct*'a (pointer) çevirmiş olabilir.
             // Eğer base ExpressionAST'in resolvedSemanticType'ı StructType ise,
             // generateLValue çağrısı Struct'ın AllocaInst*'ını döndürmüş olabilir.

             if (!basePtrOrValue) {
                  reportCodeGenError(memberAccess->base->location, "Üye erişimi temel ifadesi (l-value) üretilemedi.");
                 return nullptr;
             }

             llvm::Value* basePtr = basePtrOrValue; // Varsayalım basePtrOrValue bir pointer.

             // Eğer basePtrOrValue bir değer ise (pointer değil), geçici bir yere store edip pointerını almanız gerekebilir.
             if (!basePtrOrValue->getType()->isPointerTy()) {
                 // Örneğin, fonksiyon döndüren struct değeri. Geçici alloca, store, sonra adres.
                 llvm::Type* baseLLVMType = basePtrOrValue->getType();
                 llvm::AllocaInst* tempAlloca = builder->CreateAlloca(baseLLVMType, nullptr, "tmp.member.base");
                 builder->CreateStore(basePtrOrValue, tempAlloca);
                 basePtr = tempAlloca;
             }


             // Base tipini al (Base ifadesinin semantik tipi)
             Type* baseSemanticType = memberAccess->base->resolvedSemanticType;
             // Eğer base tipi referans (&Struct) ise, referansın gösterdiği tipi al
             if (baseSemanticType->isReferenceType()) {
                  baseSemanticType = static_cast<ReferenceType*>(baseSemanticType)->referencedType;
             }
             // Base tipi struct olmalı.
             if (!baseSemanticType->isStructType()) {
                 reportCodeGenError(memberAccess->base->location, "Üye erişimi için struct bekleniyor.");
                 return nullptr;
             }
             const StructType* structType = static_cast<const StructType*>(baseSemanticType);

             // Üye ismini al
             const std::string& memberName = memberAccess->member->name;

             // Struct tipinden üye bilgisini bul (alanın offseti veya indexi)
             const StructFieldInfo* fieldInfo = structType->findField(memberName); // TypeSystem/StructType'ta findField olmalı.
             if (!fieldInfo) {
                 reportCodeGenError(memberAccess->member->location, "Struct '" + structType->name + "' içinde '" + memberName + "' alanı bulunamadı.");
                 return nullptr;
             }

             // LLVM GEP (GetElementPtr) komutunu kullanarak üye adresini hesapla
             // GEP instruction needs pointer type, index list. First index is 0 (for the struct itself), second is field index.
             llvm::Type* baseLLVMType = basePtr->getType()->getPointerElementType(); // Pointer'ın işaret ettiği LLVM tipi
             int fieldIndex = -1; // Alanın indexini bulmanız gerekir. StructType field listesindeki sırası.
             // StructType alan listesini gezerek fieldInfo'nun indexini bulun.
              for(size_t i = 0; i < structType->fields.size(); ++i) {
                  if (&structType->fields[i] == fieldInfo) {
                      fieldIndex = i;
                      break;
                  }
              }
             if (fieldIndex == -1) {
                  reportCodeGenError(memberAccess->member->location, "Struct alanı '" + memberName + "' indexi bulunamadı.");
                  return nullptr;
             }

             return builder->CreateStructGEP(baseLLVMType, basePtr, fieldIndex, memberName.c_str()); // Üye adresini döndür

        } else if (IndexAccessAST* indexAccess = dynamic_cast<IndexAccessAST*>(expr)) {
            // Index erişiminin sol tarafı (l-value)
            // Base ifadesini l-value olarak üret (pointerını al)
            llvm::Value* basePtr = generateLValue(indexAccess->base.get()); // Dizi/Pointer'ın adresini al
            if (!basePtr) {
                 reportCodeGenError(indexAccess->base->location, "Index erişimi temel ifadesi (l-value) üretilemedi.");
                return nullptr;
            }

            // Base tipini al (Array*, Pointer*, Reference to Array/Pointer)
            Type* baseSemanticType = indexAccess->base->resolvedSemanticType;
            // Eğer base tipi referans (&Array) ise, referansın gösterdiği tipi al
            if (baseSemanticType->isReferenceType()) {
                 baseSemanticType = static_cast<ReferenceType*>(baseSemanticType)->referencedType;
            }

            llvm::Type* baseLLVMType = basePtr->getType()->getPointerElementType(); // Pointer'ın işaret ettiği LLVM tipi

            // Index ifadesini üret (tamsayı olmalı)
            llvm::Value* indexVal = generateExpression(indexAccess->index.get());
            if (!indexVal) {
                 reportCodeGenError(indexAccess->index->location, "Index ifadesi üretilemedi.");
                 return nullptr;
            }
             // Index tipinin tamsayı olduğunu SEMA garanti etti.
             // Index değeri, pointer hesabında kullanılacak integer tipinde olmalı (genellikle i64 veya size_t).
             // Eğer indexVal'in tipi i32 ise ve hedef 64-bit ise, i64'e genişletin.
             if (indexVal->getType()->isIntegerTy()) {
                 if (indexVal->getType()->getIntegerBitWidth() < builder->getInt64Ty()->getBitWidth()) {
                     indexVal = builder->CreateSExt(indexVal, builder->getInt64Ty(), "index.sext"); // Signed Extend
                 } else if (indexVal->getType()->getIntegerBitWidth() > builder->getInt64Ty()->getBitWidth()) {
                     reportCodeGenError(indexAccess->index->location, "Index ifadesi çok büyük tamsayı tipinde."); // Çok büyükse hata veya truncation
                       indexVal = builder->CreateTrunc(indexVal, builder->getInt64Ty(), "index.trunc");
                 }
             } else {
                 reportCodeGenError(indexAccess->index->location, "Index ifadesi tamsayı tipinde olmalı.");
                 return nullptr;
             }


            // LLVM GEP (GetElementPtr) komutunu kullanarak eleman adresini hesapla
            // Dizi için: GEP ArrayType*, BasePointer, Index1(0), Index2(elem_index)
            // Pointer için: GEP PointerType*, BasePointer, Index1(elem_index)

            llvm::Value* elementPtr = nullptr;
             if (baseSemanticType->isArrayType()) {
                 // Base pointer bir dizi tipine pointer (&[T; Size] veya &[T])
                 // GEP instruction: pointer to array, index 0 (to get to the first element), index into the elements
                 elementPtr = builder->CreateInBoundsGEP(baseLLVMType, basePtr, {builder->getInt64(0), indexVal}, "array.elem.ptr");
             } else if (baseSemanticType->isReferenceType() && baseSemanticType->toString().find("[") != std::string::npos) { // &[] gibi slice
                 // Base pointer bir slice'a (pointer + length struct) pointer ise...
                 // Slices için GEP mantığı farklıdır (pointer üyesinden GEP yapılır).
                 reportCodeGenError(indexAccess->base->location, "Slice index erişimi implemente edilmedi.");
                 return nullptr;
             } else if (baseLLVMType->isPointerTy()) {
                  // Base pointer bir T* pointer
                 // GEP instruction: pointer to T, index
                  elementPtr = builder->CreateInBoundsGEP(baseLLVMType, basePtr, indexVal, "pointer.elem.ptr");
             }
            // Diğer koleksiyon tipleri...

            if (!elementPtr) {
                 reportCodeGenError(indexAccess->location, "Index erişim pointerı üretilemedi.");
                 return nullptr;
            }

             return elementPtr; // Eleman adresini döndür

        }

        // Diğer ifade türleri (Literaller, BinaryOp, UnaryOp vb.) l-value değildir.
        reportCodeGenError(expr->location, "Kod üretimi için l-value olmayan ifade türü: " + expr->getNodeType());
        return nullptr; // L-value olmayan ifade için null
    }

    // Bir değeri kopyalamak için IR üretir
     void LLVMCodeGenerator::generateCopy(llvm::Value* sourceValue, llvm::Value* targetPtr, Type* valueType) {
        if (!sourceValue || !targetPtr || !valueType) return;

        llvm::Type* valueLLVMType = getLLVMType(valueType);
        if (!valueLLVMType) {
            reportCodeGenError(TokenLocation(), "Kopyalama için LLVM tip üretilemedi.");
            return;
        }

        // Kaynak bir değer (r-value) ise, hedef adrese (targetPtr) store yap.
        if (!sourceValue->getType()->isPointerTy()) {
            // Type'lar eşleşmeli (veya SEMA uyumluluğu kontrol etti).
            // Eğer tipler tam eşleşmiyorsa (örn: i32 -> float), çevirim komutu eklemeniz gerekebilir.
            if (sourceValue->getType() != valueLLVMType) {
                 // Otomatik çevirim kuralları veya explicit cast IR'ı burada üretilebilir.
                  if (valueType->isIntType() && targetPtr->getType()->getPointerElementType()->isFloatingPointTy()) { sourceValue = builder->CreateSIToFP(sourceValue, targetPtr->getType()->getPointerElementType()); }
                 reportCodeGenError(TokenLocation(), "Kopyalama sırasında kaynak ve hedef tip uyumsuzluğu."); // SEMA kontrol etti varsayılıyor
                 return;
            }
             builder->CreateStore(sourceValue, targetPtr); // Değeri adrese yaz (store)
        }
        // Kaynak bir pointer (l-value) ise, adresteki değeri yükle (load) ve hedef adrese store yap.
        // Veya daha verimli olarak memcpy kullan. Memcpy daha geneldir ve büyük yapılar için daha iyi.
        else { // sourceValue->getType()->isPointerTy()
             // İki pointer aynı tipe işaret etmeli (veya uyumlu olmalı)
             if (sourceValue->getType() != targetPtr->getType()) {
                  reportCodeGenError(TokenLocation(), "Kopyalama sırasında kaynak ve hedef pointer tipi uyumsuzluğu."); // SEMA kontrol etti varsayılıyor
                  return;
             }

             // memcpy komutunu kullan (sourceValue'dan hedef adresine)
             llvm::Value* typeSize = getCNTTypeSize(valueType); // CNT tipinin boyutunu al
             builder->CreateMemCpy(targetPtr, llvm::MaybeAlign(), sourceValue, llvm::MaybeAlign(), typeSize);
        }
    }

    // Bir değeri taşımak için IR üretir (memcpy)
    void LLVMCodeGenerator::generateMove(llvm::Value* sourcePtr, llvm::Value* targetPtr, Type* valueType) {
        if (!sourcePtr || !targetPtr || !valueType) return;

        llvm::Type* valueLLVMType = getLLVMType(valueType);
        if (!valueLLVMType) {
            reportCodeGenError(TokenLocation(), "Taşıma için LLVM tip üretilemedi.");
            return;
        }

        // Taşıma işlemi, kaynağın içeriğini hedefe kopyalamak ve kaynağı geçersiz kılmaktır.
        // LLVM IR'de bu genellikle memcpy ile yapılır. Kaynağın geçersiz kılınması SEMA/OwnershipChecker tarafından takip edilir.
        // Kaynak ve hedef pointerları olmalı.
        if (!sourcePtr->getType()->isPointerTy() || !targetPtr->getType()->isPointerTy()) {
            reportCodeGenError(TokenLocation(), "Taşıma işlemi için kaynak ve hedef pointer olmalı.");
            return;
        }
         // Pointer tipleri aynı olmalı (veya uyumlu olmalı).
         if (sourcePtr->getType() != targetPtr->getType()) {
             reportCodeGenError(TokenLocation(), "Taşıma sırasında kaynak ve hedef pointer tipi uyumsuzluğu."); // SEMA kontrol etti varsayılıyor
               llvm::Type* targetPointee = targetPtr->getType()->getPointerElementType();
               llvm::Type* sourcePointee = sourcePtr->getType()->getPointerElementType();
              // Eğer tipler uyumluysa (örn: farklı Struct Type objeleri ama aynı LLVM temsili), bitcast gerekebilir.
               sourcePtr = builder->CreateBitCast(sourcePtr, targetPtr->getType(), "move.src.bitcast");
         }

        // Memcpy komutunu üret
        llvm::Value* typeSize = getCNTTypeSize(valueType); // CNT tipinin boyutunu al
        builder->CreateMemCpy(targetPtr, llvm::MaybeAlign(), sourcePtr, llvm::MaybeAlign(), typeSize);

        // Kaynağın geçersiz kılınması (poison value atama veya null olarak işaretleme)
        // Bu genellikle gerekli değildir veya SEMA/OwnershipChecker tarafından mantıksal olarak yapılır.
        // IR seviyesinde kaynak adresi genellikle hala var olur.
    }

    // Bir değeri Drop etmek için IR üretir (destructor çağrısı)
    void LLVMCodeGenerator::generateDrop(llvm::Value* valuePtr, Type* valueType) {
        if (!valuePtr || !valueType || !currentLLVMFunction) return;

        // Eğer tip Drop implementasyonu gerektiriyorsa (hasDrop TypeSystem'den gelir)
        if (typeSystem.hasDrop(valueType)) {
            // Drop fonksiyonunu bul veya oluştur (örn: "__drop_" + type->toString() gibi bir isimle)
            // Bu drop fonksiyonları ya built-in (String) ya da kullanıcı tanımlı struct/enumlar için compiler tarafından üretilir.
            // Compiler tarafından üretilen drop fonksiyonları, alanların/varyantların drop edilmesi gerekenleri özyinelemeli olarak çağırır.
            // Drop fonksiyonunun imzası genellikle void fn(*mut Type) şeklindedir.

            // Drop fonksiyonunun LLVM Function objesini SymbolTable'dan veya özel bir haritadan bulmanız gerekir.
            / llvm::Function* dropFunc = getDropFunction(valueType); // Yardımcı metod
            llvm::Function* dropFunc = nullptr; // Placeholder

            if (!dropFunc) {
                // Drop fonksiyonu bulunamadı veya üretilmedi. Hata raporla.
                 reportCodeGenError(TokenLocation(), "Drop fonksiyonu bulunamadı veya üretilmedi: " + typeSystem.serializeType(valueType));
                return;
            }

            // Drop fonksiyonunu çağır (değerin adresini argüman olarak ver)
            // valuePtr, drop edilecek değerin konumunun pointerı olmalı.
             // Drop fonksiyonunun beklediği pointer tipi valuePtr'ın tipiyle aynı olmalı veya cast edilebilir olmalı.
             llvm::Type* requiredArgType = dropFunc->getFunctionType()->getParamType(0);
             if (valuePtr->getType() != requiredArgType) {
                 // Pointer tipini çevir (bitcast)
                 valuePtr = builder->CreateBitCast(valuePtr, requiredArgType, "drop.arg.bitcast");
             }

            builder->CreateCall(dropFunc, valuePtr);
        }
        // Copy olan veya Drop implementasyonu olmayan tipler için Drop IR'ı üretilmez.
    }


    // Belirli bir kapsam dışına çıkan değişkenler için Drop IR'ı üretir
    void LLVMCodeGenerator::generateDropsForScopeExit(const Scope* scope) {
        if (!scope || !currentLLVMFunction || !builder->GetInsertBlock()) return;

        // Bu kapsamda tanımlanmış olan değişkenleri bul.
        // SymbolTable::getSymbolsInScope(scope) gibi bir helper kullanılabilir.
        // OwnershipChecker::variableStatuses map'ini kullanarak değişkenin durumunu kontrol et.
        // Eğer durum Owned ise ve Drop gerektiriyorsa, generateDrop çağrısı yap.
        // Değişkenler ters sırada (tanımlandıklarının tersi) Drop edilmelidir.

        // Örneğin:
         std::vector<const SymbolInfo*> variablesInScope = symbolTable.getVariablesInScope(scope); // Helper SymbolTable'dan
         std::reverse(variablesInScope.begin(), variablesInScope.end()); // Ters sırada drop

         for (const auto* varSymbol : variablesInScope) {
             auto status_it = ownershipChecker.variableStatuses.find(varSymbol); // OwnershipChecker durumuna erişim
             if (status_it != ownershipChecker.variableStatuses.end()) {
                 VariableStatus finalStatus = status_it->second;
                 if (finalStatus == VariableStatus::Owned) {
                     // Kapsam çıkışında hala Owned durumda. Drop edilmesi gerekiyorsa Drop IR'ı üret.
                     if (typeSystem.hasDrop(varSymbol->type)) {
                          llvm::Value* varPtr = getSymbolValue(varSymbol); // Değişkenin alloca adresini al
                          if (varPtr) {
                              generateDrop(varPtr, varSymbol->type); // Drop IR'ı üret
                          }
                     }
                 }
        //         // Moved, Borrowed, Dropped durumları burada drop edilmez.
             }
         }
    }


    // Bir CNT semantik Type* objesini karşılık gelen LLVM llvm::Type*'a çevirir (cache kullanarak)
    llvm::Type* LLVMCodeGenerator::getLLVMType(Type* type) {
        // mapCNTTypeToLLVMType metodunu çağırır. Ayrı bir wrapper sağlandı.
        return mapCNTTypeToLLVMType(type);
    }

    // Struct Layout hesapla (Alan offsetleri için)
    void LLVMCodeGenerator::computeStructLayout(StructType* structType) {
        if (!structType || !dataLayout) return;

        llvm::Type* llvmStructType = getLLVMType(structType); // LLVM Type'ı al
        if (!llvmStructType || !llvmStructType->isStructTy() || llvmStructType->isOpaqueTy()) {
            // LLVM tipi oluşturulamadı, opaque veya bir hata. Layout hesaplayamayız.
            reportCodeGenError(TokenLocation(), "Struct '" + structType->name + "' için LLVM tipi oluşturulamadı veya opaque.");
            return;
        }

        // LLVM'in DataLayout objesini kullanarak alan offsetlerini hesapla
        const llvm::StructLayout* layout = dataLayout->getStructLayout(static_cast<llvm::StructType*>(llvmStructType));

        // Hesaplanan offsetleri StructType'ın StructFieldInfo'larına kaydet
        for (size_t i = 0; i < structType->fields.size(); ++i) {
            structType->fields[i].offset = layout->getElementOffset(i);
             diagnostics.reportInfo(TokenLocation(), "Struct '" + structType->name + "' Alan '" + structType->fields[i].name + "' Offset: " + std::to_string(structType->fields[i].offset)); // Debug
        }
    }

    // Enum Layout hesapla (Diskriminant değerleri vb.)
    void LLVMCodeGenerator::computeEnumLayout(EnumType* enumType) {
         if (!enumType) return;
        // Enum layoutu genellikle basit integer temel tipine veya variant data içeren struct/union'a dayanır.
        // Diskriminant değerleri atanır (varsa).
        // SEMA veya TypeSystem'in EnumType objesi üzerinde bu bilgileri saklaması gerekir.
        // CodeGen burada sadece SEMA'nın belirlediği diskriminantları kullanır.
    }


    // Diğer üretim metodlarının implementasyonları (generateStatement alt metodları, generateExpression alt metodları)
    // Bu metodlar, ilgili AST düğümünün yapısını gezerek (recursive çağrılar),
    // SEMA tarafından eklenmiş anlamsal bilgileri (resolvedSemanticType, resolvedSymbol vb.) kullanarak,
    // LLVM builder (this->builder) ile uygun LLVM IR komutlarını (CreateAdd, CreateCall, CreateBr vb.) üretir.
    // Bu komutların detaylı implementasyonu, CNT dilinin her özelliğinin LLVM'ye nasıl çevrileceğine bağlıdır.
    // Örneğin, generateMatchExpression SwitchInst ve Branch komutlarını kullanır.
    // generateMemberAccess GEP komutunu kullanır.
    // generateAssignment, generateMove/generateCopy/generateStore komutlarını kullanır.

} // namespace cnt_compiler
