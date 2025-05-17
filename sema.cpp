#include "sema.h"
#include <iostream> // Debug çıktıları için
#include <typeinfo> // dynamic_cast için typeid

// Semantic Analyzer Kurucu
SemanticAnalyzer::SemanticAnalyzer(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, OwnershipChecker& oc, ModuleResolver& mr)
    : diagnostics(diag), typeSystem(ts), symbolTable(st), ownershipChecker(oc), moduleResolver(mr) {
    // Temel tipleri sembol tablosuna ekle (int, float, bool vb.)
    // Bu, typeSystem.getIntType() gibi çağrılarla TypeSystem içinde de yapılabilir ve sembol tablosuna SEMA'nın ilk geçişinde eklenebilir.
    // Örneğin:
     auto intTypeSymbol = std::make_shared<SymbolInfo>("int", typeSystem.getIntType(), nullptr, false);
     symbolTable.insert("int", intTypeSymbol);
    // ... diğer temel tipler
}

// Semantik analizi başlatan ana metod
bool SemanticAnalyzer::analyze(ProgramAST* program) {
    // Global kapsamı oluştur
    symbolTable.enterScope(); // Global kapsam

    // İlk Geçiş: Bildirimleri işle (İsimleri sembol tablosuna ekle ve tipleri çöz)
    // Bu, fonksiyonların/structların ileri bildirimlerine (forward declaration) izin verir.
    // Fonksiyonların imzaları, struct ve enumların isimleri bu aşamada bilinir hale getirilir.
    for (const auto& decl_ptr : program->declarations) {
        analyzeDeclaration(decl_ptr.get()); // Bildirimleri analiz et (bu recursive çağrı başlatır)
    }

    // İkinci Geçiş (Gerekliyse): Fonksiyon gövdelerini ve başlangıç değerlerini analiz et
    // Artık tüm isimler sembol tablosunda olduğu için çözümleme yapılabilir ve tip çıkarımı tamamlanabilir.
    analyzeProgram(program); // analyzeDeclaration zaten tüm ağacı geziyor, bu ikinci bir geçiş olabilir.
                               // Veya analyzeDeclaration içinde sadece isim ve imza kaydı yapıp,
                               // analyzeProgram içinde gövdeleri işleyen ayrı bir pass yapabilirsiniz.
                               // Mevcut yapı analyzeDeclaration'ı tam olarak işleyecek şekilde tasarlandı.

    // Sahiplik, Ödünç Alma ve Yaşam Süreleri Kontrolü (AST üzerinde ayrı bir geçiş gerektirebilir)
     ownershipChecker.check(program); // Sahiplik/Ödünç Alma kurallarını uygula

    // Anlamsal hatalar bulundu mu?
    return diagnostics.hasErrors();
}

// Genel düğüm analizcisi (Türüne göre dallanma)
void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (!node) return;

    std::cout << "Analyzing Node: " << node->getNodeType() << std::endl; // Debug

    // RTTI (Run-Time Type Information) kullanarak düğüm türüne göre dallanma
    if (ProgramAST* p = dynamic_cast<ProgramAST*>(node)) {
        analyzeProgram(p);
    } else if (DeclarationAST* d = dynamic_cast<DeclarationAST*>(node)) {
        analyzeDeclaration(d);
    } else if (StatementAST* s = dynamic_cast<StatementAST*>(node)) {
        analyzeStatement(s);
    } else if (ExpressionAST* e = dynamic_cast<ExpressionAST*>(node)) {
        analyzeExpression(e); // İfade analizcileri tip döndürür, base metod resolvedSemanticType'ı ayarlar.
    } else if (TypeAST* t = dynamic_cast<TypeAST*>(node)) {
        analyzeTypeAST(t); // Tip analizcileri semantik tip döndürür, base metod resolvedSemanticType'ı ayarlar.
    }
    // MatchArmAST gibi diğer özel AST düğüm türleri
    else if (MatchArmAST* arm = dynamic_cast<MatchArmAST*>(node)) {
        // MatchArm'ın paternini ve sonucunu analiz et
        analyzeNode(arm->pattern.get()); // Pattern analizi (daha sonra PatternAST türleri tanımlanınca detaylanır)
        analyzeExpression(arm->result.get());
    }
     else {
        // Bilinmeyen düğüm türü (Olması gereken bir durum değil)
        diagnostics.reportInternalError(node->location, "Anlamsal analiz için bilinmeyen AST düğümü türü: " + std::string(typeid(*node).name()));
    }
}

// Programın tamamını analiz et
void SemanticAnalyzer::analyzeProgram(ProgramAST* program) {
    // Bildirimleri analiz et (Fonksiyon gövdeleri, başlangıç değerleri vb.)
    // Bu döngü aslında ana analyze(program) metodundaki ilk geçişe denk gelebilir.
    // İkinci bir geçiş olarak düşünülüyorsa, burada sadece gövdeleri analiz etme mantığı olmalı.
    // analyzeDeclaration metodu zaten recursive, bu yüzden analyze(program) içindeki tek döngü yeterli.
    // Bu metod sadece global kapsamdan çıkışı yönetmek için kullanılabilir.

    // analyze(program) metodunda global kapsam açıldı.
    // Tüm analiz bittikten sonra global kapsam kapanmalı.
     symbolTable.exitScope(); // Bu çağrı analyze(program) sonunda yapılmalı.
}

// Bildirim analizcisi (Türüne göre dallanır)
void SemanticAnalyzer::analyzeDeclaration(DeclarationAST* decl) {
     if (!decl) return;
    std::cout << "Analyzing Declaration: " << decl->getNodeType() << std::endl; // Debug

    // 'pub' anahtar kelimesinin doğru yerde kullanılıp kullanılmadığını kontrol et.
    // Parser isPublic bayrağını set etti. SEMA, bu bildirim türünün public olmaya uygun olup olmadığını kontrol eder.
    if (decl->isPublic) {
        if (!isValidPublicDeclarationType(decl)) {
             diagnostics.reportError(decl->location, "'" + decl->getNodeType() + "' türündeki bildirim public olamaz.");
             // Hata durumunda isPublic flag'ini false yapabiliriz veya SEMA sonrası temizlikte bu hata durumunu dikkate alabiliriz.
              decl->isPublic = false; // Hata sonrası flag'i temizle
        }
    }

    // Bildirim türüne göre ilgili analizi çağır
    if (FunctionDeclAST* funcDecl = dynamic_cast<FunctionDeclAST*>(decl)) {
        analyzeFunctionDecl(funcDecl);
    } else if (StructDeclAST* structDecl = dynamic_cast<StructDeclAST*>(decl)) {
        analyzeStructDecl(structDecl);
    } else if (EnumDeclAST* enumDecl = dynamic_cast<EnumDeclAST*>(decl)) {
        analyzeEnumDecl(enumDecl);
    } else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(decl)) {
         // VarDeclAST hem Declaration hem Statement'tan miras alıyor.
         // parseStatement() içinde yerel değişkenleri de yakaladığımız için, burada sadece global değişkenleri analiz edebiliriz.
         // Veya analyzeVarDecl metoduna isGlobal flag'i geçirebiliriz.
         // VarDeclAST yapısına isGlobal üyesi eklemek daha temiz olabilir.
         // Varsayalım VarDeclAST'te isGlobal üyesi var ve parser tarafından set ediliyor.
          analyzeVarDecl(varDecl); // analyzeVarDecl içinde isGlobal ve isPublic'i kontrol etsin.
         // Alternatif olarak, parserdan gelen isGlobal flag'ini kullanmak için:
          analyzeVarDecl(varDecl, varDecl->isGlobal); // analyzeVarDecl'e isGlobal flag'i geçirmek.
         // Parser artık isGlobal flag'i geçirmiyor, VarDeclAST'te isGlobal yok. Parser'dan gelen bilgi sadece isPublic ve isMutable.
         // Global/Yerel ayrımı SEMA'nın kendisi scope'a bakarak yapmalı.
         analyzeVarDecl(varDecl); // analyzeVarDecl içinde scope'a bakarak global/yerel belirlenecek.
    }
    // ... Diğer bildirim türleri (trait, impl)
     else {
        diagnostics.reportInternalError(decl->location, "Anlamsal analiz için bilinmeyen bildirim türü: " + std::string(typeid(*decl).name()));
    }
}

// Fonksiyon Bildirimini Analiz Et
void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclAST* funcDecl) {
    std::cout << "Analyzing Function: " << funcDecl->name->name << std::endl; // Debug

    // Fonksiyon ismini sembol tablosuna ekle
    // Sembol türü FUNCTION_KIND olacak.
    // Fonksiyonun tipi (FunctionType) burada belirlenmeli.
    // FunctionType objesi argüman tiplerini ve dönüş tipini içerir.
    // Argüman ve dönüş tipleri önce analyzeTypeAST ile çözülmeli.
    // SEMA'nın ilk geçişinde sadece isim ve imza kaydedilir, gövde daha sonra.
    // Bu implementasyon hem isim/imza kaydı hem de gövde analizi yapıyor gibi görünüyor.

    // 1. İsim ve İmza Çözümleme (İlk Geçiş Rolü)
    // İsim mevcut scope'ta tanımlı mı?
    if (symbolTable.lookupCurrentScope(funcDecl->name->name)) {
         diagnostics.reportError(funcDecl->name->location, "İsim '" + funcDecl->name->name + "' bu kapsamda zaten tanımlı.");
         // Hata sonrası sembol eklemeyebiliriz, ancak gövde analizine devam etmek diğer hataları bulmak için iyi olabilir.
          return; // İsim çakışması ciddiyse durdur.
    }

    // Argüman ve dönüş tiplerini analiz et (TypeAST -> semantic Type*)
    std::vector<Type*> paramTypes;
    for (const auto& arg_ptr : funcDecl->arguments) {
        Type* paramSemanticType = analyzeTypeAST(arg_ptr->type.get());
        if (!paramSemanticType || paramSemanticType->id == Type::ERROR_TYPE) {
            // Hata zaten raporlandı.
             paramSemanticType = typeSystem.getErrorType(); // Hata durumunda ERROR_TYPE kullan
        }
         arg_ptr->type->resolvedSemanticType = paramSemanticType; // AST düğümüne semantik tipi ekle (Zaten analyzeTypeAST yapıyor olmalı)
        paramTypes.push_back(paramSemanticType);

        // Argüman sembolünü fonksiyona özel bir geçici kapsamda veya SymbolInfo içinde sakla
         SymbolInfo* argSymbol = new SymbolInfo(arg_ptr->name->name, paramSemanticType, arg_ptr.get(), arg_ptr->isMutable);
         arg_ptr->resolvedSymbol = argSymbol; // AST düğümüne sembol bilgisini ekle
        // Bu semboller fonksiyon kapsamına eklenmelidir.
    }

    Type* returnSemanticType = typeSystem.getVoidType(); // Varsayılan dönüş tipi void
    if (funcDecl->returnType) {
        returnSemanticType = analyzeTypeAST(funcDecl->returnType.get());
        if (!returnSemanticType || returnSemanticType->id == Type::ERROR_TYPE) {
             returnSemanticType = typeSystem.getErrorType();
        }
         funcDecl->returnType->resolvedSemanticType = returnSemanticType; // AST düğümüne semantik tipi ekle
    }

    // Fonksiyonun semantik tipini oluştur (TypeSystem kullan)
     Type* functionSemanticType = typeSystem.getFunctionType(returnSemanticType, paramTypes); // TypeSystem'de FunctionType oluşturma metodu olmalı.

    // Fonksiyon için SymbolInfo oluştur ve sembol tablosuna ekle
     auto funcSymbol = std::make_shared<SymbolInfo>(funcDecl->name->name, functionSemanticType, funcDecl, false); // Fonksiyonlar mutable olmaz
     symbolTable.insert(funcDecl->name->name, funcSymbol);
     funcDecl->resolvedSymbol = funcSymbol.get(); // AST düğümüne sembol bilgisini ekle

    // 2. Gövde Analizi (İkinci Geçiş Rolü veya İlk Geçişin Devamı)
    if (funcDecl->body) {
        // currentFunctionReturnType ve inFunction durumlarını ayarla
        Type* oldFunctionReturnType = currentFunctionReturnType;
        bool oldInFunction = inFunction;
        currentFunctionReturnType = returnSemanticType;
        inFunction = true;

        // Fonksiyon kapsamına gir (parametreler burada sembol tablosuna eklenir)
        symbolTable.enterScope();
        // Argüman sembollerini fonksiyon kapsamına ekle
        // ... argüman SymbolInfo'larını symbolTable.insert() ile ekle ...

        // Fonksiyon gövdesini analiz et
        analyzeBlockStatement(funcDecl->body.get());

        // Kapsam sonu (varsa temizlik)
         ownershipChecker.endScope(symbolTable.getCurrentScope()); // OwnershipChecker'a bildir

        // Fonksiyon kapsamından çık
        symbolTable.exitScope();

        // currentFunctionReturnType ve inFunction durumlarını geri yükle
        currentFunctionReturnType = oldFunctionReturnType;
        inFunction = oldInFunction;
    }
}

// Struct Bildirimini Analiz Et
void SemanticAnalyzer::analyzeStructDecl(StructDeclAST* structDecl) {
    std::cout << "Analyzing Struct: " << structDecl->name->name << std::endl; // Debug

    // İsim mevcut scope'ta tanımlı mı?
    if (symbolTable.lookupCurrentScope(structDecl->name->name)) {
         diagnostics.reportError(structDecl->name->location, "İsim '" + structDecl->name->name + "' bu kapsamda zaten tanımlı.");
         return;
    }

    // Struct için semantik tipi Tip Sistemine kaydet (Tam alan bilgisiyle birlikte)
    // Bu, recursive veya ileri bildirimli tipleri yönetmeye yardımcı olur.
     StructType* structSemanticType = typeSystem.registerStructType(structDecl); // Tip Sistemi bu struct'ı kaydetsin ve Type* dönsün.

    // Struct için SymbolInfo oluştur ve sembol tablosuna ekle (Tip: structSemanticType)
     auto structSymbol = std::make_shared<SymbolInfo>(structDecl->name->name, structSemanticType, structDecl, false); // Structlar mutable olmaz
     symbolTable.insert(structDecl->name->name, structSymbol);
     structDecl->resolvedSymbol = structSymbol.get(); // AST düğümüne sembol bilgisini ekle
     structDecl->resolvedSemanticType = structSemanticType; // AST düğümüne semantik tipi ekle

    // Alanları analiz et (isimleri ve tipleri)
    // Alanlar için SymbolInfo oluşturulabilir veya structSemanticType içinde alan bilgisi tutulabilir.
    // Alanların tipleri analyzeTypeAST ile çözülmeli.
    // Struct alanları genellikle struct'ın kendi kapsamı içinde ele alınmaz sembol tablosunda,
    // ancak üye erişimi sırasında structSemanticType bilgisi kullanılır.
    for (const auto& field_ptr : structDecl->fields) {
        Type* fieldSemanticType = analyzeTypeAST(field_ptr->type.get());
        if (!fieldSemanticType || fieldSemanticType->id == Type::ERROR_TYPE) {
             fieldSemanticType = typeSystem.getErrorType();
        }
         field_ptr->type->resolvedSemanticType = fieldSemanticType; // AST düğümüne semantik tipi ekle

          structSemanticType->addField(field_ptr->name->name, fieldSemanticType); // Tip Sistemindeki StructType'a alanı ekle
          field_ptr->resolvedSemanticType = fieldSemanticType; // Alan düğümüne tipini ekle
          field_ptr->resolvedSymbol = ... // Alan için symbol info? (Opsiyonel)
    }
}

// Enum Bildirimini Analiz Et
void SemanticAnalyzer::analyzeEnumDecl(EnumDeclAST* enumDecl) {
    // Benzer şekilde struct gibi, ismi sembol tablosuna ekle, tipi Tip Sistemine kaydet, varyantları işle.
    // Variyantlar için SymbolInfo oluşturulabilir veya EnumType içinde varyant bilgisi tutulabilir.
     if (symbolTable.lookupCurrentScope(enumDecl->name->name)) {
         diagnostics.reportError(enumDecl->name->location, "İsim '" + enumDecl->name->name + "' bu kapsamda zaten tanımlı.");
         return;
    }

     EnumType* enumSemanticType = typeSystem.registerEnumType(enumDecl); // Tip Sistemi bu enum'ı kaydetsin.

     auto enumSymbol = std::make_shared<SymbolInfo>(enumDecl->name->name, enumSemanticType, enumDecl, false);
     symbolTable.insert(enumDecl->name->name, enumSymbol);
     enumDecl->resolvedSymbol = enumSymbol.get();
     enumDecl->resolvedSemanticType = enumSemanticType;

    // Varyantları işle
    for (const auto& variant_ptr : enumDecl->variants) {
        // Varyant isimleri enum kapsamında benzersiz olmalı.
         varyant için SymbolInfo oluşturup enumSemanticType içinde saklayabilirsiniz.
         varyant_ptr->resolvedSymbol = ...;
         varyant_ptr->resolvedSemanticType = ...; // İlişkili tipleri varsa
         enumSemanticType->addVariant(variant_ptr->name->name, ...);
    }
}


// Değişken Bildirimini Analiz Et (Global veya Yerel)
void SemanticAnalyzer::analyzeVarDecl(VarDeclAST* varDecl) {
    // Bu metod, sembol tablosu kapsamına bakarak değişkenin global mi yerel mi olduğunu belirleyebilir.
    bool isGlobal = (symbolTable.getCurrentScopeDepth() == 1); // Global scope depth'in 1 olduğunu varsayalım.

    // İsim mevcut kapsamda tanımlı mı?
    if (symbolTable.lookupCurrentScope(varDecl->name->name)) {
         diagnostics.reportError(varDecl->name->location, "İsim '" + varDecl->name->name + "' bu kapsamda zaten tanımlı.");
         return;
    }

    // Tip belirtilmişse analiz et, yoksa başlangıç değerinden çıkarım yap.
    Type* varType = nullptr;
    if (varDecl->type) {
        varType = analyzeTypeAST(varDecl->type.get());
        if (!varType || varType->id == Type::ERROR_TYPE) {
            varType = typeSystem.getErrorType();
        }
          varDecl->type->resolvedSemanticType = varType; // AnalyzeTypeAST zaten bunu yapıyor olmalı.
    }

    Type* initializerType = nullptr;
    if (varDecl->initializer) {
        initializerType = analyzeExpression(varDecl->initializer.get()); // analyzeExpression resolvedSemanticType'ı AST'ye yazar.
        if (!initializerType || initializerType->id == Type::ERROR_TYPE) {
             initializerType = typeSystem.getErrorType();
        } else {
            if (!varType) { // Tip çıkarımı
                 varType = initializerType;
            } else { // Tip kontrolü (atanabilirlik)
                 if (!typeSystem.isAssignable(initializerType, varType, varDecl->isMutable)) {
                     diagnostics.reportError(varDecl->initializer->location,
                                             "'" + initializerType->toString() + "' tipi '" + varType->toString() + "' tipine atanamaz.");
                     varType = typeSystem.getErrorType();
                 }
            }
        }
    } else {
        // Başlangıç değeri yoksa ve tip belirtilmemişse hata (CNT kuralı)
        if (!varType) {
             diagnostics.reportError(varDecl->location,
                                     "Değişken '" + varDecl->name->name + "' başlangıç değeri veya tip belirtilmeden tanımlanamaz.");
             varType = typeSystem.getErrorType();
        }
    }

     if (varType->id == Type::ERROR_TYPE) {
        // Hata varsa sembolü eklemeyebiliriz veya ERROR_TYPE ile ekleriz.
        // ERROR_TYPE ile eklemek, sonraki aşamalarda da hatayı yaymasını sağlar.
         SymbolInfo* errorSymbol = new SymbolInfo(varDecl->name->name, varType, varDecl, varDecl->isMutable);
         varDecl->resolvedSymbol = errorSymbol;
         symbolTable.insert(varDecl->name->name, std::shared_ptr<SymbolInfo>(errorSymbol));
        return; // Hata durumunda devam etme (Sembol eklemeyi atla)
     }


    // Değişken bilgisini sembol tablosuna ekle
    auto symbol = std::make_shared<SymbolInfo>(varDecl->name->name, varType, varDecl, varDecl->isMutable);
    // SymbolInfo'ya isGlobal ve isPublic bilgisini de eklemek isteyebilirsiniz.
     symbol->isGlobal = isGlobal;
     symbol->isPublic = varDecl->isPublic; // isPublic bayrağı AST'de varDecl->isPublic'ten alınır.

    symbolTable.insert(varDecl->name->name, symbol);

    // Değişkenin AST düğümüne çözümlenmiş sembol bilgisini ekle (Annotation)
    varDecl->resolvedSymbol = symbol.get();

    // Sahiplik Kuralları: Yeni tanımlanan değişken başlangıçta değere sahip olur (OWNED)
     ownershipChecker.recordVariableDeclaration(varDecl, varType, varDecl->isMutable, varDecl->location);
}


// Deyim analizcisi (Türüne göre dallanır)
void SemanticAnalyzer::analyzeStatement(StatementAST* stmt) {
     if (!stmt) return;
    std::cout << "Analyzing Statement: " << stmt->getNodeType() << std::endl; // Debug

    if (BlockStatementAST* block = dynamic_cast<BlockStatementAST*>(stmt)) {
        analyzeBlockStatement(block);
    } else if (ExpressionStatementAST* exprStmt = dynamic_cast<ExpressionStatementAST*>(stmt)) {
        analyzeExpression(exprStmt->expression.get()); // İfadeyi analiz et (resolvedSemanticType AST'ye yazılır)
        // Bir deyim ifadesinin sonucu genellikle yoksayılır, ancak OwnershipChecker'ın geçici değerleri yönetmesi gerekebilir.
         ownershipChecker.handleTemporaryValue(exprStmt->expression->resolvedSemanticType, exprStmt->location);
    } else if (ImportStatementAST* importStmt = dynamic_cast<ImportStatementAST*>(stmt)) {
        analyzeImportStatement(importStmt); // Import'u analiz et
    } else if (ReturnStatementAST* returnStmt = dynamic_cast<ReturnStatementAST*>(stmt)) {
        analyzeReturnStatement(returnStmt); // Return'ü analiz et
    } else if (BreakStatementAST* breakStmt = dynamic_cast<BreakStatementAST*>(stmt)) {
        analyzeBreakStatement(breakStmt);   // Break'i analiz et
    } else if (ContinueStatementAST* continueStmt = dynamic_cast<ContinueStatementAST*>(stmt)) {
        analyzeContinueStatement(continueStmt); // Continue'yu analiz et
    } else if (WhileStatementAST* whileStmt = dynamic_cast<WhileStatementAST*>(stmt)) {
         analyzeWhileStatement(whileStmt); // While'ı analiz et
    }
    // IfStatementAST veya ForStatementAST gibi diğer deyim türleri
    // VarDeclAST hem Declaration hem Statement olduğu için burada yerel değişkenler yakalanır:
    else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
         // Yerel değişken bildirimi olarak analiz et
         analyzeVarDecl(varDecl); // analyzeVarDecl içinde scope'a bakarak yerel olduğu anlaşılacak.
    }
     else {
        diagnostics.reportInternalError(stmt->location, "Anlamsal analiz için bilinmeyen deyim türü: " + std::string(typeid(*stmt).name()));
    }
}

// Blok Deyimi Analiz Et
void SemanticAnalyzer::analyzeBlockStatement(BlockStatementAST* block) {
     if (!block) return;
    // Yeni bir kapsam oluştur ve gir
    symbolTable.enterScope();
     ownershipChecker.enterScope(); // OwnershipChecker'a scope girildiğini bildir

    // Blok içindeki deyimleri analiz et
    for (const auto& stmt_ptr : block->statements) {
        analyzeStatement(stmt_ptr.get());
    }

    // Kapsam sonu temizliği (drop çağrıları, ödünç almaların sonu)
     ownershipChecker.endScope(symbolTable.getCurrentScope()); // OwnershipChecker'a bildir

    // Kapsamdan çık
    symbolTable.exitScope();
}

// Import Deyimi Analiz Et
void SemanticAnalyzer::analyzeImportStatement(ImportStatementAST* importStmt) {
     if (!importStmt) return;
    // ModuleResolver'ı kullanarak import'u çözümle
    // ModuleResolver, `.hnt` dosyasını bulacak, ayrıştıracak ve import edilen sembolleri
    // *şu anki* sembol tablosuna veya bir alias altında ekleyecektir.
    // analyze() metodunda açılan global scope'u ModuleResolver'a geçirmemiz gerekebilir.
    // analyzeImportStatement, bir fonksiyon veya blok içindeki import'ları da yönetebilmeli.
    // ModuleResolver::resolveImportStatement metodunun hangi scope'a import edileceğini parametre olarak alması daha esnektir.

    bool success = moduleResolver.resolveImportStatement(
        importStmt,
        symbolTable // İmport edilen sembollerin ekleneceği sembol tablosu
    );

    if (success) {
        // Eğer çözümleme başarılıysa, çözümlenen ModuleInterface'i AST düğümüne ekle.
        // ModuleResolver resolveImportStatement metodundan ModuleInterface* veya shared_ptr döndürebilir
        // veya importStmt->resolvedInterface alanını kendisi doldurabilir.
        // Varsayalım ModuleResolver importStmt->resolvedInterface'i dolduruyor.
        if (!importStmt->resolvedInterface) {
            // ModuleResolver başarılı döndü ama resolvedInterface null? Internal hata.
            diagnostics.reportInternalError(importStmt->location, "Import çözümlemesi başarılı ancak resolvedInterface null.");
        }
        // Başarı durumunda ek bir işlem yapmaya gerek yok, semboller zaten sembol tablosunda.
    } else {
        // Hata durumunda (modül bulunamadı vb.) ModuleResolver zaten hata raporladı.
         importStmt->resolvedInterface = nullptr; // Hata durumunda null olarak kalır.
    }
}

// Return Deyimi Analiz Et
void SemanticAnalyzer::analyzeReturnStatement(ReturnStatementAST* returnStmt) {
     if (!returnStmt) return;
    // Fonksiyon içinde miyiz kontrol et
    if (!inFunction) {
        diagnostics.reportError(returnStmt->location, "'return' deyimi sadece fonksiyon içinde kullanılabilir.");
        return;
    }

    // Dönüş değeri varsa, ifadesini analiz et ve tipini al
    Type* returnValueType = nullptr;
    if (returnStmt->returnValue) {
        returnValueType = analyzeExpression(returnStmt->returnValue.get());
        if (!returnValueType || returnValueType->id == Type::ERROR_TYPE) {
            returnValueType = typeSystem.getErrorType(); // Hata zaten raporlandı
        }
    } else {
        // Dönüş değeri yoksa, tipi void
        returnValueType = typeSystem.getVoidType();
    }

    // Dönüş değeri tipinin fonksiyonun beklenen dönüş tipine uyumlu olup olmadığını kontrol et (atanabilirlik)
    // currentFunctionReturnType SEMA durumu içinde tutuluyor.
    if (currentFunctionReturnType && !typeSystem.isAssignable(returnValueType, currentFunctionReturnType, false)) { // Fonksiyon dönüş tipi mutable olmaz.
         diagnostics.reportError(returnStmt->location,
                                 "Fonksiyon dönüş tipi uyumsuz. Beklenen: '" + currentFunctionReturnType->toString() + "', Bulunan: '" + returnValueType->toString() + "'.");
    }

    // Sahiplik Kuralları: Return, local değişkenlerin scope'tan çıkmasına neden olabilir.
     ownershipChecker.handleReturn(returnStmt); // Return durumunda temizlik vs.
}

// Break Deyimi Analiz Et
void SemanticAnalyzer::analyzeBreakStatement(BreakStatementAST* breakStmt) {
     if (!breakStmt) return;
    // Döngü içinde miyiz kontrol et
    if (!inLoop) {
        diagnostics.reportError(breakStmt->location, "'break' deyimi sadece döngü içinde kullanılabilir.");
        return;
    }
    // Etiket (label) varsa çözümlemesi ve kontrolü yapılacak.

    // Sahiplik Kuralları: Break, inner scope'lardan çıkmaya neden olur.
     ownershipChecker.handleBreak(breakStmt); // Break durumunda temizlik vs.
}

// Continue Deyimi Analiz Et
void SemanticAnalyzer::analyzeContinueStatement(ContinueStatementAST* continueStmt) {
     if (!continueStmt) return;
     // Döngü içinde miyiz kontrol et
    if (!inLoop) {
        diagnostics.reportError(continueStmt->location, "'continue' deyimi sadece döngü içinde kullanılabilir.");
        return;
    }
     // Etiket (label) varsa çözümlemesi ve kontrolü yapılacak.

    // Sahiplik Kuralları: Continue, mevcut döngü iterasyonunun sonuna atlar, scope'tan çıkmaya neden olabilir.
     ownershipChecker.handleContinue(continueStmt); // Continue durumunda temizlik vs.
}

// While Döngüsü Analiz Et
void SemanticAnalyzer::analyzeWhileStatement(WhileStatementAST* whileStmt) {
     if (!whileStmt) return;
    // Koşul ifadesini analiz et ve tipinin bool olduğunu kontrol et.
    Type* conditionType = analyzeExpression(whileStmt->condition.get());
    if (!conditionType || conditionType->id == Type::ERROR_TYPE) {
         conditionType = typeSystem.getErrorType();
    }

    if (conditionType->id != Type::ERROR_TYPE && conditionType->id != Type::BOOL_TYPE) {
        diagnostics.reportError(whileStmt->condition->location,
                                "While döngüsü koşulu boolean tipinde olmalı. Bulunan: '" + conditionType->toString() + "'.");
    }

    // Döngü gövdesini analiz etmeden önce 'inLoop' durumunu ayarla
    bool oldInLoop = inLoop;
    inLoop = true;

    // Döngü gövdesini analiz et (bu bir bloktur)
    analyzeBlockStatement(whileStmt->body.get()); // analyzeBlockStatement kendi kapsamını açar/kapatır.

    // 'inLoop' durumunu geri yükle
    inLoop = oldInLoop;

    // Sahiplik Kuralları: Döngü giriş/çıkışları, iterasyonlar arası geçişler sahiplik/ödünç alma durumlarını etkiler.
     ownershipChecker.analyzeLoop(whileStmt); // Döngüye özel analiz
}


// İfade analizcisi (Tipini döndürür ve AST düğümüne yazar)
Type* SemanticAnalyzer::analyzeExpression(ExpressionAST* expr) {
     if (!expr) return typeSystem.getErrorType();

    std::cout << "Analyzing Expression: " << expr->getNodeType() << std::endl; // Debug

    Type* exprType = typeSystem.getErrorType(); // Varsayılan hata tipi

    // Düğüm türüne göre alt analizciyi çağır
    if (IntLiteralAST* lit = dynamic_cast<IntLiteralAST*>(expr)) {
        exprType = analyzeIntLiteral(lit);
    } else if (FloatLiteralAST* lit = dynamic_cast<FloatLiteralAST*>(expr)) {
        exprType = analyzeFloatLiteral(lit);
    } else if (StringLiteralAST* lit = dynamic_cast<StringLiteralAST*>(expr)) {
        exprType = analyzeStringLiteral(lit);
    } else if (CharLiteralAST* lit = dynamic_cast<CharLiteralAST*>(expr)) {
        exprType = analyzeCharLiteral(lit);
    } else if (BoolLiteralAST* lit = dynamic_cast<BoolLiteralAST*>(expr)) {
        exprType = analyzeBoolLiteral(lit);
    } else if (IdentifierAST* id = dynamic_cast<IdentifierAST*>(expr)) {
        exprType = analyzeIdentifier(id); // Identifier özel çünkü isim çözümlemesi yapar
    } else if (BinaryOpAST* binOp = dynamic_cast<BinaryOpAST*>(expr)) {
        exprType = analyzeBinaryOp(binOp);
    } else if (UnaryOpAST* unOp = dynamic_cast<UnaryOpAST*>(expr)) {
        exprType = analyzeUnaryOp(unOp);
    } else if (AssignmentAST* assign = dynamic_cast<AssignmentAST*>(expr)) {
        exprType = analyzeAssignment(assign);
    } else if (CallExpressionAST* call = dynamic_cast<CallExpressionAST*>(expr)) {
        exprType = analyzeCallExpression(call);
    } else if (MemberAccessAST* memberAccess = dynamic_cast<MemberAccessAST*>(expr)) {
        exprType = analyzeMemberAccess(memberAccess);
    } else if (IndexAccessAST* indexAccess = dynamic_cast<IndexAccessAST*>(expr)) {
        exprType = analyzeIndexAccess(indexAccess);
    } else if (MatchExpressionAST* matchExpr = dynamic_cast<MatchExpressionAST*>(expr)) {
         exprType = analyzeMatchExpression(matchExpr);
    }
    // Parantezli ifade gibi diğer türler...
     else if (ParenthesizedExpressionAST* parenExpr = dynamic_cast<ParenthesizedExpressionAST*>(expr)) {
        exprType = analyzeExpression(parenExpr->inner.get()); // İç ifadeyi analiz et
     }

    else {
         diagnostics.reportInternalError(expr->location, "Anlamsal analiz için bilinmeyen ifade türü: " + std::string(typeid(*expr).name()));
        exprType = typeSystem.getErrorType();
    }

    // Belirlenen semantik tipi AST düğümüne ekle (Annotation)
    expr->resolvedSemanticType = exprType;

    // Sahiplik Kuralları: İfade sonucu üretilen geçici değerler.
     ownershipChecker.handleExpressionResult(expr, exprType); // Opsiyonel: Geçici değer yönetimini bildir.

    return exprType; // Semantik tipi döndür
}

// Identifier Analiz Et (İsim Çözümleme)
Type* SemanticAnalyzer::analyzeIdentifier(IdentifierAST* identifier) {
    if (!identifier) return typeSystem.getErrorType();

    // İsimi sembol tablosunda ara (mevcut ve üst kapsamlarda)
    auto symbol = symbolTable.lookup(identifier->name);

    if (!symbol) {
        diagnostics.reportError(identifier->location, "'" + identifier->name + "' ismi tanımlı değil.");
        identifier->resolvedSymbol = nullptr;
        return typeSystem.getErrorType(); // Hata tipi döndür
    }

    // AST düğümüne çözümlenen sembol bilgisini ekle (Annotation)
    identifier->resolvedSymbol = symbol.get();

    // Sembolün tipini al (SymbolInfo'da saklanıyor)
    Type* symbolType = symbol->type;

    // AST düğümüne çözümlenen semantik tipi ekle (Annotation)
    // resolvedSemanticType ExpressionAST'ten miras alındığı için IdentifierAST'te de var.
     identifier->resolvedSemanticType = symbolType; // analyzeExpression base metodu bunu yapacak.

    // Sahiplik Kuralları: Değişken kullanımı (load, move, borrow)
     ownershipChecker.handleVariableUse(symbol.get(), identifier->location, UseKind::Load); // Kullanım türünü belirleyin (Okuma, Yazma, Taşıma, Ödünç Alma)


    return symbolType; // Sembolün tipini döndür
}

// Binary Operatör İfade Analiz Et (+, -, ==, && vb.)
Type* SemanticAnalyzer::analyzeBinaryOp(BinaryOpAST* binaryOp) {
    if (!binaryOp) return typeSystem.getErrorType();

    // Sol ve sağ operand ifadeleri analiz et ve tiplerini al (analyzeExpression resolvedSemanticType'ı AST'ye yazar)
    Type* leftType = analyzeExpression(binaryOp->left.get());
    Type* rightType = analyzeExpression(binaryOp->right.get());

    // Eğer operand tipleri hata tipi ise, sonuç da hata tipidir.
    if (leftType->id == Type::ERROR_TYPE || rightType->id == Type::ERROR_TYPE) {
         binaryOp->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType();
    }

    // Operand tiplerine ve operatöre göre tip uyumluluğunu ve sonuç tipini belirle
    Type* resultType = typeSystem.getErrorType(); // Varsayılan olarak hata tipi

    // Burada operatör aşırı yüklemesi (operator overloading) mantığı da devreye girebilir.
    // Şimdilik sadece temel tipler için built-in operatörleri kontrol edelim.

    switch (binaryOp->op) {
        case Token::TOK_PLUS:
        case Token::TOK_MINUS:
        case Token::TOK_STAR:
        case Token::TOK_SLASH:
        case Token::TOK_PERCENT:
            // Aritmetik operatörler
            if (typeSystem.areTypesEqual(leftType, rightType) && (leftType->id == Type::INT_TYPE || leftType->id == Type::FLOAT_TYPE)) {
                 resultType = leftType; // Sonuç tipi operandlarla aynı
            } else {
                 diagnostics.reportError(binaryOp->location,
                                         "'" + binaryOp->lexeme + "' operatörü için uyumsuz operand tipleri: '" + leftType->toString() + "' ve '" + rightType->toString() + "'.");
            }
            break;
        case Token::TOK_EQ:
        case Token::TOK_NE:
        case Token::TOK_LT:
        case Token::TOK_GT:
        case Token::TOK_LE:
        case Token::TOK_GE:
            // Karşılaştırma operatörleri
            if (typeSystem.areTypesEqual(leftType, rightType) && (leftType->id == Type::INT_TYPE || leftType->id == Type::FLOAT_TYPE || leftType->id == Type::STRING_TYPE)) { // String karşılaştırması ekleyebilirsiniz
                 resultType = typeSystem.getBoolType(); // Sonuç bool
            } else {
                 diagnostics.reportError(binaryOp->location,
                                         "'" + binaryOp->lexeme + "' operatörü için uyumsuz operand tipleri: '" + leftType->toString() + "' ve '" + rightType->toString() + "'.");
            }
            break;
        case Token::TOK_AND:
        case Token::TOK_OR:
            // Mantıksal operatörler (sadece bool)
            if (leftType->id == Type::BOOL_TYPE && rightType->id == Type::BOOL_TYPE) {
                resultType = typeSystem.getBoolType();
            } else {
                diagnostics.reportError(binaryOp->location,
                                        "'" + binaryOp->lexeme + "' operatörü sadece boolean tipleri için geçerlidir. Bulunan: '" + leftType->toString() + "' ve '" + rightType->toString() + "'.");
            }
            break;
        // ... Diğer ikili operatörler (bitişik atamalar += vb.)

        default:
             diagnostics.reportInternalError(binaryOp->location, "Anlamsal analiz için bilinmeyen ikili operatör.");
            break;
    }

    // binaryOp->resolvedSemanticType = resultType; // analyzeExpression base metodu yapacak
    return resultType; // Sonuç tipini döndür
}

// Unary Operatör İfade Analiz Et (!, -, &, &mut, *)
Type* SemanticAnalyzer::analyzeUnaryOp(UnaryOpAST* unaryOp) {
    if (!unaryOp) return typeSystem.getErrorType();

    // Operand ifadeyi analiz et ve tipini al
    Type* operandType = analyzeExpression(unaryOp->operand.get()); // analyzeExpression resolvedSemanticType'ı AST'ye yazar.

    if (operandType->id == Type::ERROR_TYPE) {
         unaryOp->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType(); // Operand hata tipi ise sonuç da hata
    }

    Type* resultType = typeSystem.getErrorType(); // Varsayılan hata tipi

    switch (unaryOp->op) {
        case Token::TOK_NOT: // Mantıksal DEĞİL (!)
            if (operandType->id == Type::BOOL_TYPE) {
                resultType = typeSystem.getBoolType();
            } else {
                 diagnostics.reportError(unaryOp->location,
                                        "Mantıksal DEĞİL (!) operatörü sadece boolean tipleri için geçerlidir. Bulunan: '" + operandType->toString() + "'.");
            }
            break;
        case Token::TOK_MINUS: // Negatif (-)
             if (operandType->id == Type::INT_TYPE || operandType->id == Type::FLOAT_TYPE) {
                resultType = operandType; // Sonuç aynı tipte
            } else {
                 diagnostics.reportError(unaryOp->location,
                                        "Negatif (-) operatörü sadece sayısal tipler için geçerlidir. Bulunan: '" + operandType->toString() + "'.");
            }
            break;
        case Token::TOK_AND: // & (Immutable Referans)
            // Herhangi bir T tipi için &T referansı oluşturur.
            // Operandın bir l-value (atanabilir değer) olması gerekir.
            // AST düğümünde l-value bilgisini tutmak gerekebilir veya SEMA burada l-value kontrolü yapmalıdır.
             if (!unaryOp->operand->isAssignableLocation) { // AST düğümünde isAssignableLocation gibi bir flag olabilir
                 diagnostics.reportError(unaryOp->operand->location, "Referans (&) alınamaz (geçici değer veya r-value).");
            //     // resultType error kalır
             } else {
                resultType = typeSystem.getReferenceType(operandType, false); // false: mutable değil

                // Sahiplik Kuralları: Bu bir ödünç alma işlemidir. OwnershipChecker'a bildir.
                // operand AST düğümünün çözülmüş sembolünü veya kendisini geçirmeniz gerekebilir.
                // SymbolInfo* operandSymbol = dynamic_cast<IdentifierAST*>(unaryOp->operand.get()) ? dynamic_cast<IdentifierAST*>(unaryOp->operand.get())->resolvedSymbol : nullptr; // Sadece Identifier ise sembolü al
                 ownershipChecker.checkBorrow(unaryOp->operand.get(), operandSymbol, false, unaryOp->location); // Mutable olmayan ödünç alma kontrolü
             }
            break;
        case Token::TOK_MUT: // &mut (Mutable Referans)
            // Herhangi bir T tipi için &mut T referansı oluşturur.
            // Operandın bir l-value OLMALI ve ayrıca değiştirilebilir (mutable) bir değişken veya konum olmalı.
             if (!unaryOp->operand->isAssignableLocation) {
                 diagnostics.reportError(unaryOp->operand->location, "Mutable referans (&mut) alınamaz (geçici değer veya r-value).");
            //     // resultType error kalır
             } else if (!unaryOp->operand->isMutableLocation) { // isMutableLocation gibi bir flag/kontrol
                 diagnostics.reportError(unaryOp->operand->location, "Mutable referans (&mut) alınamaz (değiştirilemez değer).");
                  // resultType error kalır
             }
             else {
                 resultType = typeSystem.getReferenceType(operandType, true); // true: mutable

                 // Sahiplik Kuralları: Mutable ödünç alma. ownershipChecker'a bildir.
                  SymbolInfo* operandSymbol = dynamic_cast<IdentifierAST*>(unaryOp->operand.get()) ? dynamic_cast<IdentifierAST*>(unaryOp->operand.get())->resolvedSymbol : nullptr;
                  ownershipChecker.checkBorrow(unaryOp->operand.get(), operandSymbol, true, unaryOp->location); // Mutable ödünç alma kontrolü (tek mutable borç kuralı)
            // }
            break;
        case Token::TOK_STAR: // * (Dereference)
            // Operandın bir pointer veya referans tipi olması gerekir.
            if (operandType->id == Type::REFERENCE_TYPE) { // || operandType->id == Type::POINTER_TYPE
                ReferenceType* refType = static_cast<ReferenceType*>(operandType);
                resultType = refType->referencedType; // Referansın gösterdiği tipi döndür

                // Sahiplik Kuralları: Dereference ile ödünç alınan değere erişim yapılıyor.
                // Kullanımın ödünç alma kurallarına uygun olduğunu kontrol et.
                 ownershipChecker.checkDereference(unaryOp->operand.get(), unaryOp->location); // Ödünç alınan değer hala geçerli mi?
            } else {
                 diagnostics.reportError(unaryOp->location,
                                        "Dereference (*) operatörü sadece referans veya pointer tipleri için geçerlidir. Bulunan: '" + operandType->toString() + "'.");
            }
            break;

        default:
             diagnostics.reportInternalError(unaryOp->location, "Anlamsal analiz için bilinmeyen tekli operatör.");
            break;
    }

     unaryOp->resolvedSemanticType = resultType; // analyzeExpression base metodu yapacak
    return resultType; // Sonuç tipini döndür
}

// Atama İfadesi Analiz Et (=)
Type* SemanticAnalyzer::analyzeAssignment(AssignmentAST* assignment) {
     if (!assignment) return typeSystem.getErrorType();
    // Sağ taraf ifadeyi analiz et ve tipini al
    Type* rightType = analyzeExpression(assignment->right.get()); // resolvedSemanticType AST'ye yazılır

    // Sol taraf ifadenin bir l-value (atanabilir konum) olduğunu kontrol et.
    // generateExpression(assignment->left.get()) çağırmak yerine, sol tarafın kendisini analiz edip l-value olduğunu doğrulamak gerekir.
    // analyzeAssignableLocation(assignment->left.get()) gibi bir metodunuz olabilir.
    // Şimdilik sol tarafı da expression gibi analiz edip, sonra tipini ve l-value/mutable durumunu kontrol edelim.

    Type* leftType = analyzeExpression(assignment->left.get()); // resolvedSemanticType AST'ye yazılır

    // Sol tarafın bir l-value olduğunu SEMA garantilemeli.
    // AST düğümlerine l-value bilgisini ekleyebilirsiniz (örn: expr->isLValue).
     if (!assignment->left->isLValue) {
          diagnostics.reportError(assignment->left->location, "Atama ifadesinin sol tarafı atanabilir bir konum değil.");
          leftType = typeSystem.getErrorType(); // Hata durumunda sol tipi hata yap
     }

    if (leftType->id == Type::ERROR_TYPE || rightType->id == Type::ERROR_TYPE) {
        // assignment->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType(); // Operand hatası varsa sonuç hata
    }

    // Sol tarafın atanabilir (mutable) olduğunu kontrol et
    // Sol tarafın SymbolInfo'su veya kaynağı (Variable, Field, Indexed Element) bilinmeli.
    // Identifier ise, SymbolInfo'sundan isMutable bayrağına bakılır.
    // MemberAccess veya IndexAccess ise, o konumun mutable olup olmadığına bakılır.
     bool isLeftMutable = checkMutableLocation(assignment->left.get()); // Mutable olup olmadığını kontrol eden yardımcı

    // Sağ taraf tipinin sol taraf tipine atanabilirliğini kontrol et (isAssignable metodunu kullan)
    // isAssignable metodunuz mutability'yi de dikkate almalı.
     if (!typeSystem.isAssignable(rightType, leftType, isLeftMutable)) {
          diagnostics.reportError(assignment->location,
                                  "'" + rightType->toString() + "' tipi '" + leftType->toString() + "' tipine atanamaz.");
    //     // Hata durumunda sonuç tipi hata olabilir veya sol taraf tipi olabilir.
          assignment->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
         return typeSystem.getErrorType();
     }

    // Sahiplik Kuralları: Atama, sağ taraftaki değerin sahipliğini sol tarafa taşıyabilir veya kopyalayabilir.
     ownershipChecker.handleAssignment(assignment->left.get(), assignment->right.get(), assignment->location);

    // Atama ifadesi genellikle atanan değerin tipini döndürür.
     assignment->resolvedSemanticType = leftType; // analyzeExpression base metodu yapacak
    return leftType;
}


// Fonksiyon Çağrısı Analiz Et
Type* SemanticAnalyzer::analyzeCallExpression(CallExpressionAST* call) {
     if (!call) return typeSystem.getErrorType();

    // Çağrılan ifadeyi analiz et (Bu genellikle bir IdentifierAST veya MemberAccessAST olur)
    Type* calleeType = analyzeExpression(call->callee.get()); // resolvedSemanticType AST'ye yazılır

    if (calleeType->id == Type::ERROR_TYPE) {
          call->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType(); // Callee hatası varsa sonuç hata
    }

    // Callee tipinin bir fonksiyon tipi (FunctionType*) olup olmadığını kontrol et.
    // Veya pointer/referans to function type.
     if (calleeType->id != Type::FUNCTION_TYPE) {
         diagnostics.reportError(call->callee->location, "Çağrılan ifade bir fonksiyon veya fonksiyon pointerı/referansı değil.");
           call->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
         return typeSystem.getErrorType();
     }
     FunctionType* funcType = static_cast<FunctionType*>(calleeType); // veya pointer/referansın gösterdiği tip

    // Callee'nin SymbolInfo'sunu al (Eğer callee IdentifierAST ise)
    SymbolInfo* calleeSymbol = dynamic_cast<IdentifierAST*>(call->callee.get()) ? dynamic_cast<IdentifierAST*>(call->callee.get())->resolvedSymbol : nullptr;
    // veya callee MemberAccessAST ise resolvedMemberSymbol'unu alabilirsiniz.
     call->resolvedCalleeSymbol = calleeSymbol; // AST'ye SymbolInfo'yu ekle

    // Argüman ifadelerini analiz et ve tiplerini topla
    std::vector<Type*> argTypes;
    for (const auto& arg_ptr : call->arguments) {
        Type* argType = analyzeExpression(arg_ptr.get()); // resolvedSemanticType AST'ye yazılır
        if (!argType || argType->id == Type::ERROR_TYPE) {
            argType = typeSystem.getErrorType(); // Hata durumunda ERROR_TYPE kullan
        }
        argTypes.push_back(argType);
    }

    // Fonksiyon çağrısının doğru olup olmadığını kontrol et (argüman sayısı ve tipi uyumluluğu)
    // SymbolInfo veya FunctionType bilgisi kullanılarak yapılır.
     checkFunctionCall(call, calleeSymbol); // veya checkFunctionCall(call, funcType);

    // Fonksiyonun dönüş tipini al (Bu, çağrı ifadesinin sonucunun tipidir)
     Type* returnType = funcType->returnType; // FunctionType'ta dönüş tipi olmalı.

    // Sahiplik Kuralları: Fonksiyon çağrısı argümanların sahipliğini/ödünç almasını yönetir.
    // Dönüş değeri de sahiplik kurallarına tabidir.
     ownershipChecker.handleFunctionCall(call, calleeSymbol, argTypes, returnType);

     call->resolvedSemanticType = returnType; // analyzeExpression base metodu yapacak
     return returnType; // Fonksiyonun dönüş tipini döndür
     return typeSystem.getVoidType(); // Placeholder dönüş tipi
}


// Üye Erişimi Analiz Et (obj.field, obj.method())
Type* SemanticAnalyzer::analyzeMemberAccess(MemberAccessAST* memberAccess) {
     if (!memberAccess) return typeSystem.getErrorType();

    // Base ifadesini analiz et ve tipini al
    Type* baseType = analyzeExpression(memberAccess->base.get()); // resolvedSemanticType AST'ye yazılır

    if (baseType->id == Type::ERROR_TYPE) {
          memberAccess->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType(); // Base hatası varsa sonuç hata
    }

    // Base tipinin bir Struct, Enum veya pointer/referans to Struct/Enum olup olmadığını kontrol et.
     Type* actualBaseType = baseType;
     if (baseType->id == Type::REFERENCE_TYPE) { actualBaseType = static_cast<ReferenceType*>(baseType)->referencedType; }
     else if (baseType->id == Type::POINTER_TYPE) { actualBaseType = static_cast<PointerType*>(baseType)->pointeeType; }

     if (actualBaseType->id != Type::STRUCT_TYPE && actualBaseType->id != Type::ENUM_TYPE) {
          diagnostics.reportError(memberAccess->base->location, "Üye erişimi için struct veya enum bekleniyor. Bulunan: '" + baseType->toString() + "'.");
           memberAccess->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
         return typeSystem.getErrorType();
     }

    // Üye ismini al
    const std::string& memberName = memberAccess->member->name;

    // Base tipinin tanımında üyeyi ara (StructType veya EnumType içinde üye arama metotları olmalı)
     StructType* structType = static_cast<StructType*>(actualBaseType); // Veya EnumType*
     SymbolInfo* memberSymbol = structType->findMember(memberName); // TypeSystem veya Type objesi içinde helper

     if (!memberSymbol) {
          diagnostics.reportError(memberAccess->member->location, "'" + actualBaseType->toString() + "' içinde '" + memberName + "' isimli üye bulunamadı.");
          memberAccess->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
         return typeSystem.getErrorType();
     }

    // Üyenin SymbolInfo'sunu AST düğümüne ekle
     memberAccess->resolvedMemberSymbol = memberSymbol;

    // Üyenin tipini al ve döndür (Bu, üye erişimi ifadesinin sonucunun tipidir)
    // Type* memberType = memberSymbol->type;
     memberAccess->resolvedSemanticType = memberType; // analyzeExpression base metodu yapacak
     return memberType;

    // Sahiplik Kuralları: Üye erişimi ödünç alma gerektirebilir veya sahipliği taşıyabilir.
     ownershipChecker.handleMemberAccess(memberAccess, baseType, memberSymbol);
     return typeSystem.getVoidType(); // Placeholder dönüş tipi
}

// Index Erişimi Analiz Et (array[index])
Type* SemanticAnalyzer::analyzeIndexAccess(IndexAccessAST* indexAccess) {
     if (!indexAccess) return typeSystem.getErrorType();

    // Base ifadesini analiz et (dizi veya pointer olmalı)
    Type* baseType = analyzeExpression(indexAccess->base.get()); // resolvedSemanticType AST'ye yazılır

    // Index ifadesini analiz et (tamsayı olmalı)
    Type* indexType = analyzeExpression(indexAccess->index.get()); // resolvedSemanticType AST'ye yazılır

    if (baseType->id == Type::ERROR_TYPE || indexType->id == Type::ERROR_TYPE) {
         indexAccess->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType();
    }

    // Base tipinin bir dizi, pointer veya referans to dizi/pointer olup olmadığını kontrol et.
    // Index tipinin tamsayı (int) olup olmadığını kontrol et.

    // Eğer base tip bir dizi ise, eleman tipini al ve döndür.
     if (baseType->id == Type::ARRAY_TYPE) {
         ArrayType* arrayType = static_cast<ArrayType*>(baseType);
         Type* elementType = arrayType->elementType; // ArrayType içinde eleman tipi olmalı.
         indexAccess->resolvedSemanticType = elementType; // analyzeExpression base metodu yapacak
         return elementType;
     } else {
          diagnostics.reportError(indexAccess->base->location, "Index erişimi için dizi bekleniyor. Bulunan: '" + baseType->toString() + "'.");
           indexAccess->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
          return typeSystem.getErrorType();
     }

    // Sahiplik Kuralları: Index erişimi ödünç alma gerektirebilir veya sahipliği taşıyabilir.
     ownershipChecker.handleIndexAccess(indexAccess, baseType, indexType);
     return typeSystem.getVoidType(); // Placeholder dönüş tipi
}


// Match İfadesi Analiz Et
Type* SemanticAnalyzer::analyzeMatchExpression(MatchExpressionAST* matchExpr) {
     if (!matchExpr) return typeSystem.getErrorType();

    // Eşleşmenin yapılacağı ifadeyi analiz et ve tipini al
    Type* valueType = analyzeExpression(matchExpr->value.get()); // resolvedSemanticType AST'ye yazılır

    if (valueType->id == Type::ERROR_TYPE) {
         matchExpr->resolvedSemanticType = typeSystem.getErrorType(); // analyzeExpression base metodu yapacak
        return typeSystem.getErrorType();
    }

    Type* commonResultType = nullptr; // Tüm kolların ortak sonucu tipi

    // Her match kolunu analiz et
    for (const auto& arm_ptr : matchExpr->arms) {
        // Kolun pattern'ını analiz et (Paternlerin anlamsal analizi karmaşık bir konudur, ayrı bir sistem gerektirebilir)
        // Pattern analizi, pattern'ın valueType ile uyumlu olup olmadığını kontrol etmeli
        // ve pattern'daki değişken bağlamalarını mevcut kapsama eklemelidir (Geçici olarak arm'ın scope'unda).
         analyzePattern(arm_ptr->pattern.get(), valueType, symbolTable.getCurrentScope());

        // Kolun sonucunu üreten ifadeyi analiz et
        Type* armResultType = analyzeExpression(arm_ptr->result.get()); // resolvedSemanticType AST'ye yazılır

        if (!armResultType || armResultType->id == Type::ERROR_TYPE) {
             // Hata zaten raporlandı. Ortak tip belirlemede hata tipi kullan.
             armResultType = typeSystem.getErrorType();
        }

        // İlk kol ise, ortak tipi bu kolun tipi olarak ayarla.
        if (!commonResultType) {
            commonResultType = armResultType;
        } else {
            // Sonraki kolların tipi, ilk kolun tipiyle uyumlu (veya aynı) olmalı.
            // Veya ortak bir ataya yükseltilebilmeli.
             if (!typeSystem.areTypesEqual(armResultType, commonResultType)) {
                 // Ortak atayı bulmaya çalış
                  Type* ancestor = typeSystem.getCommonAncestor(armResultType, commonResultType);
                  if (ancestor) {
                      commonResultType = ancestor;
                  } else {
                     // Uyumsuz kollar, hata raporla
                     diagnostics.reportError(arm_ptr->result->location,
                                             "Match kollarının tipleri uyumsuz. '" + armResultType->toString() + "' tipi önceki kolların tipiyle uyumlu değil.");
                     commonResultType = typeSystem.getErrorType(); // Hata durumunda ortak tipi hata yap
                  }
             }
        }

         // Sahiplik Kuralları: Pattern içindeki değişken bağlamaları, koldaki ifade, kapsam.
          ownershipChecker.handleMatchArm(arm_ptr.get());
    }

    // Match kapsamlılık (exhaustiveness) kontrolü: valueType'ın tüm olası durumları paternlerle eşleşiyor mu?
     checkMatchExhaustiveness(matchExpr, valueType);

    // Match erişilebilirlik (reachability) kontrolü: Herhangi bir kol kendisinden önceki bir kol tarafından tamamen kapsanıyor mu?
     checkMatchReachability(matchExpr);

     matchExpr->resolvedSemanticType = commonResultType ? commonResultType : typeSystem.getVoidType(); // Değer döndürmüyorsa void olabilir. analyzeExpression base metodu yapacak.
    return commonResultType ? commonResultType : typeSystem.getVoidType();
}


// Tip AST düğümlerini analiz eder (Semantik tip objesine pointer döndürür ve AST düğümüne yazar)
Type* SemanticAnalyzer::analyzeTypeAST(TypeAST* typeNode) {
    if (!typeNode) {
          typeNode->resolvedSemanticType = typeSystem.getErrorType(); // Base metod yapacak
        return typeSystem.getErrorType();
    }

    Type* semanticType = typeSystem.getErrorType(); // Varsayılan hata tipi

    // Düğüm türüne göre alt analizciyi çağır
    if (BaseTypeAST* baseType = dynamic_cast<BaseTypeAST*>(typeNode)) {
        semanticType = analyzeBaseTypeAST(baseType);
    } else if (ReferenceTypeAST* refType = dynamic_cast<ReferenceTypeAST*>(typeNode)) {
        semanticType = analyzeReferenceTypeAST(refType);
    } else if (ArrayTypeAST* arrayType = dynamic_cast<ArrayTypeAST*>(typeNode)) {
        semanticType = analyzeArrayTypeAST(arrayType);
    }
    // PointerTypeAST, TupleTypeAST gibi diğer türler...

    else {
         diagnostics.reportInternalError(typeNode->location, "Anlamsal analiz için bilinmeyen tip AST düğümü türü: " + std::string(typeid(*typeNode).name()));
    }

    // Belirlenen semantik tipi AST düğümüne ekle (Annotation)
    typeNode->resolvedSemanticType = semanticType;

    return semanticType; // Semantik tipi döndür
}

// Base Type AST Analiz Et (int, string, MyStruct gibi)
Type* SemanticAnalyzer::analyzeBaseTypeAST(BaseTypeAST* baseTypeNode) {
     if (!baseTypeNode) return typeSystem.getErrorType();

    // Temel tipi çözümle: "int", "float" gibi ana tipler veya kullanıcı tanımlı (struct, enum) isimleri
    if (baseTypeNode->name == "int") return typeSystem.getIntType();
    else if (baseTypeNode->name == "float") return typeSystem.getFloatType();
    else if (baseTypeNode->name == "bool") return typeSystem.getBoolType();
    else if (baseTypeNode->name == "string") return typeSystem.getStringType();
    else if (baseTypeNode->name == "char") return typeSystem.getCharType();
    else if (baseTypeNode->name == "void") return typeSystem.getVoidType(); // void tip AST'de olabilir (örn: fn() -> void)
    // ... diğer temel tipler

    else {
        // Kullanıcı tanımlı tip (Struct, Enum, Trait adı olmalı)
        // İsimi sembol tablosunda ara. Bir Struct veya Enum bildirimi olmalı.
        auto symbol = symbolTable.lookup(baseTypeNode->name);
        if (symbol && symbol->type && (symbol->type->id == Type::STRUCT_TYPE || symbol->type->id == Type::ENUM_TYPE /* || symbol->type->id == Type::TRAIT_TYPE */)) {
             baseTypeNode->resolvedSymbol = symbol.get(); // Eğer BaseTypeAST'te SymbolInfo* tutuluyorsa
            return symbol->type; // Çözümlenmiş struct/enum/trait tipini kullan
        } else {
            diagnostics.reportError(baseTypeNode->location, "'" + baseTypeNode->name + "' tanımlı bir tip ismi değil.");
            return typeSystem.getErrorType();
        }
    }
}

// Reference Type AST Analiz Et (&T, &mut T)
Type* SemanticAnalyzer::analyzeReferenceTypeAST(ReferenceTypeAST* refTypeNode) {
     if (!refTypeNode) return typeSystem.getErrorType();
    // Referansın gösterdiği tipi analiz et
    Type* referencedSemanticType = analyzeTypeAST(refTypeNode->referencedType.get()); // resolvedSemanticType AST'ye yazılır

    if (!referencedSemanticType || referencedSemanticType->id == Type::ERROR_TYPE) {
         refTypeNode->resolvedSemanticType = typeSystem.getErrorType(); // Base metod yapacak
        return typeSystem.getErrorType(); // İç tip hata ise sonuç da hata
    }

    // Semantik ReferenceType objesini Tip Sisteminden al veya oluştur
    return typeSystem.getReferenceType(referencedSemanticType, refTypeNode->isMutable);
}

// Array Type AST Analiz Et ([T] veya [T; size])
Type* SemanticAnalyzer::analyzeArrayTypeAST(ArrayTypeAST* arrayTypeNode) {
     if (!arrayTypeNode) return typeSystem.getErrorType();
     // Eleman tipini analiz et
    Type* elementSemanticType = analyzeTypeAST(arrayTypeNode->elementType.get()); // resolvedSemanticType AST'ye yazılır

    if (!elementSemanticType || elementSemanticType->id == Type::ERROR_TYPE) {
         arrayTypeNode->resolvedSemanticType = typeSystem.getErrorType(); // Base metod yapacak
        return typeSystem.getErrorType(); // Eleman tipi hata ise sonuç da hata
    }

    // Sabit boyut varsa, boyut ifadesini analiz et ve tipinin int olduğunu kontrol et.
     llvm::ConstantInt* arraySize = nullptr;
     if (arrayTypeNode->size) {
        Type* sizeType = analyzeExpression(arrayTypeNode->size.get()); // resolvedSemanticType AST'ye yazılır.
        if (!sizeType || sizeType->id != Type::INT_TYPE) {
             diagnostics.reportError(arrayTypeNode->size->location, "Dizi boyutu tamsayı olmalı.");
              arrayTypeNode->resolvedSemanticType = typeSystem.getErrorType(); // Base metod yapacak
             return typeSystem.getErrorType();
        }
        // Boyut ifadesinin sabit (compile-time constant) olduğunu da kontrol etmeniz gerekebilir.
        // Eğer constant ise, değerini alıp semantic ArrayType objesine veya AST düğümüne kaydedin.
         arraySize = getConstantInt(arrayTypeNode->size.get()); // Constant değerini çıkaran yardımcı
     }


    // Semantik ArrayType objesini Tip Sisteminden al veya oluştur
     return typeSystem.getArrayType(elementSemanticType, arraySize); // Boyut bilgisiyle
     return typeSystem.getArrayType(elementSemanticType); // Boyut bilgisiz

     arrayTypeNode->resolvedSemanticType = resultType; // Base metod yapacak
     return resultType; // Oluşturulan ArrayType*
}


// =======================================================================
// Anlamsal Kontrol ve Doğrulama Yardımcı Metodları Implementasyonu
// =======================================================================

// İki semantik tipin eşit olup olmadığını kontrol et (TypeSystem kullanır)
bool SemanticAnalyzer::areTypesEqual(Type* t1, Type* t2) {
    return typeSystem.areTypesEqual(t1, t2);
}

// Bir tipin diğerine atanabilir olup olmadığını kontrol et (Mutability dikkate alınır) (TypeSystem kullanır)
bool SemanticAnalyzer::isAssignable(Type* valueType, Type* targetType, bool isTargetMutable) {
    return typeSystem.isAssignable(valueType, targetType, isTargetMutable);
}

// Fonksiyon çağrısının doğru olup olmadığını kontrol et (argüman sayısı/tipi)
void SemanticAnalyzer::checkFunctionCall(CallExpressionAST* callExpr, SymbolInfo* calleeSymbol) {
     if (!callExpr || !calleeSymbol || !calleeSymbol->type || calleeSymbol->type->id != Type::FUNCTION_TYPE) {
        // Hata zaten analyzeCallExpression içinde raporlandı veya sembol fonksiyon değil.
        return;
    }

     FunctionType* funcType = static_cast<FunctionType*>(calleeSymbol->type);

    // Argüman sayısı kontrolü (callExpr->arguments.size() vs funcType->parameterTypes.size())
    // ...

    // Argüman tipleri kontrolü (callExpr->arguments[i]->resolvedSemanticType vs funcType->parameterTypes[i])
    // isAssignable metodunu kullanarak her argüman tipinin ilgili parametre tipine atanabilirliğini kontrol edin.
    // Parametrelerin mutable olup olmadığını FunctionArgAST veya FunctionType içinde tutmanız gerekir.
    // ...
}


// Belirli bir bildirim türünün public olmasına izin verilip verilmediğini kontrol et
bool SemanticAnalyzer::isValidPublicDeclarationType(const ASTNode* node) const {
    // CNT dilinizin kurallarına göre hangi AST düğüm türlerinin (bildirimlerin) public olabileceğini belirleyin.
    // Örn: FunctionDeclAST, StructDeclAST, EnumDeclAST, VarDeclAST (eğer global ise)
    // Diğerleri (BlockStatementAST, ExpressionStatementAST gibi) public olamaz.

    if (dynamic_cast<const FunctionDeclAST*>(node)) return true;
    if (dynamic_cast<const StructDeclAST*>(node)) return true;
    if (dynamic_cast<const EnumDeclAST*>(node)) return true;
    // Global değişkenler public olabilir mi? VarDeclAST'in global olup olmadığını SEMA anlamalı.
    // Eğer analyzeVarDecl içinde isGlobal bayrağı belirleniyorsa, burada kontrol edilebilir.
     if (const VarDeclAST* varDecl = dynamic_cast<const VarDeclAST*>(node)) {
          return varDecl->isGlobal; // VarDeclAST'te isGlobal flag varsa
     }

    return false; // Varsayılan olarak public olamaz
}


// Match ifadesi için kapsamlılık ve erişilebilirlik kontrolü (OwnershipChecker veya ayrı helper)
void SemanticAnalyzer::checkMatchExhaustiveness(MatchExpressionAST* matchExpr, Type* valueType) {
    // Bu çok karmaşık bir kontroldür. valueType'ın tüm olası değerlerinin (tüm int değerleri, tüm enum varyantları, tüm bool değerleri, vb.)
    // match kollarındaki pattern'ler tarafından kapsanıp kapsanmadığını belirlemeyi içerir.
    // _ wildcard pattern'i tüm geri kalan durumları kapsar.
    // Literal pattern'ler belirli değerleri kapsar.
    // Enum varyant pattern'leri belirli varyantları kapsar.
    // Destructuring pattern'ler struct/enum'un belirli yapılarını kapsar.
    // Bu, TypeSystem bilgisine (enum varyantları, struct alanları vb.) ve Pattern analizine dayanır.
    // Genellikle ayrı bir helper fonksiyonda implemente edilir.
    diagnostics.reportWarning(matchExpr->location, "Match kapsamlılık (exhaustiveness) kontrolü implemente edilmedi."); // Placeholder
}

void SemanticAnalyzer::checkMatchReachability(MatchExpressionAST* matchExpr) {
    // Bu da karmaşık bir kontroldür. İkinci bir pattern'ın kendisinden önceki pattern(ler) tarafından
    // tamamen kapsanıp kapsanmadığını belirlemeyi içerir. Eğer kapsanıyorsa, o kola asla ulaşılamaz.
    // Kolları sırayla karşılaştırarak yapılır.
    diagnostics.reportWarning(matchExpr->location, "Match erişilebilirlik (reachability) kontrolü implemente edilmedi."); // Placeholder
}
