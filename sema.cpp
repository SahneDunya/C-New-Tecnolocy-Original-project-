#include "sema.h"
#include <iostream> // Hata ayıklama için

// Gerekli diğer başlıklar (yardımcı sistem .cpp dosyaları derleme biriminde)
 #include "symbol_table.cpp" // Genellikle ayrı derlenirler, burada include edilmez.
 #include "type_system.cpp"
 #include "ownership_checker.cpp"
 #include "module_resolver.cpp"


// Semantic Analyzer Kurucu
SemanticAnalyzer::SemanticAnalyzer(Diagnostics& diag, TypeSystem& ts, SymbolTable& st, OwnershipChecker& oc, ModuleResolver& mr)
    : diagnostics(diag), typeSystem(ts), symbolTable(st), ownershipChecker(oc), moduleResolver(mr) {
    // Kurulum işlemleri
}

// Semantik analizi başlatan ana metod
bool SemanticAnalyzer::analyze(ProgramAST* program) {
    // Global kapsamı oluştur
    symbolTable.enterScope(); // Global kapsam

    // İlk Geçiş: Bildirimleri işle (İsimleri sembol tablosuna ekle)
    // Bu, fonksiyonların/structların ileri bildirimlerine (forward declaration) izin verir.
    for (const auto& decl_ptr : program->declarations) {
        // analyzeDeclaration(decl_ptr.get()); // pointer'ı geç
        // OwnershipChecker'ın da global kapsamı bilmesi gerekebilir.
        // Şimdilik sadece sema içinden çağrıldığını varsayalım.
    }

    // İkinci Geçiş: Fonksiyon gövdelerini ve başlangıç değerlerini analiz et
    // Artık tüm isimler sembol tablosunda olduğu için çözümleme yapılabilir.
    analyzeProgram(program); // AST üzerinde traversal'ı başlat

    // Sahiplik, Ödünç Alma ve Yaşam Süreleri Kontrolü (AST üzerinde ayrı bir geçiş gerektirebilir)
     analyzeLifetimes(program); // Yaşam sürelerini analiz et (varsa)
     ownershipChecker.check(program); // Sahiplik/Ödünç Alma kurallarını uygula

    // Anlamsal hatalar bulundu mu?
    return diagnostics.hasErrors();
}

// Genel düğüm analizcisi (Türüne göre dallanma)
void SemanticAnalyzer::analyzeNode(ASTNode* node) {
    if (!node) return; // Null pointer kontrolü

    std::cout << "Analyzing Node: " << node->getNodeType() << std::endl; // Debug çıktısı

    // RTTI (Run-Time Type Information) kullanarak düğüm türüne göre dallanma
    // Daha temiz bir yapı için Visitor pattern önerilir.
    if (ProgramAST* p = dynamic_cast<ProgramAST*>(node)) {
        analyzeProgram(p);
    } else if (DeclarationAST* d = dynamic_cast<DeclarationAST*>(node)) {
        analyzeDeclaration(d);
    } else if (StatementAST* s = dynamic_cast<StatementAST*>(node)) {
        analyzeStatement(s);
    } else if (ExpressionAST* e = dynamic_cast<ExpressionAST*>(node)) {
        // İfade analizcileri tip döndürür, analyzeExpression çağrılmalı
        analyzeExpression(e); // Dönüş tipini şimdilik yoksayalım, sadece analizi tetikleyelim.
    } else if (TypeAST* t = dynamic_cast<TypeAST*>(node)) {
        analyzeTypeAST(t); // Tip düğümlerini semantik tiplere dönüştür
    }
    // ... Diğer özel AST düğüm türleri
    else {
        // Bilinmeyen düğüm türü (Olması gereken bir durum değil)
        diagnostics.reportError(node->location.filename, node->location.line, node->location.column,
                                "Anlamsal analiz için bilinmeyen AST düğümü türü.");
    }
}


// Programın tamamını analiz et
void SemanticAnalyzer::analyzeProgram(ProgramAST* program) {
    // Bildirimleri analiz et (Fonksiyon gövdeleri, başlangıç değerleri vb.)
    for (const auto& decl_ptr : program->declarations) {
        analyzeDeclaration(decl_ptr.get());
    }
    // Global kapsamdan çık (Program analizi bittiğinde)
    symbolTable.exitScope();
}

// Bildirim analizcisi (Türüne göre dallanır)
void SemanticAnalyzer::analyzeDeclaration(DeclarationAST* decl) {
    //std::cout << "Analyzing Declaration: " << decl->getNodeType() << std::endl; // Debug çıktısı
    if (FunctionDeclAST* funcDecl = dynamic_cast<FunctionDeclAST*>(decl)) {
        analyzeFunctionDecl(funcDecl);
    } else if (StructDeclAST* structDecl = dynamic_cast<StructDeclAST*>(decl)) {
        analyzeStructDecl(structDecl);
    } else if (EnumDeclAST* enumDecl = dynamic_cast<EnumDeclAST*>(decl)) {
        analyzeEnumDecl(enumDecl);
    } else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(decl)) {
        // Global değişken bildirimi olarak analiz et
        analyzeVarDecl(varDecl, true);
    }
    // ... Diğer bildirim türleri
     else {
        diagnostics.reportError(decl->location.filename, decl->location.line, decl->location.column,
                                "Anlamsal analiz için bilinmeyen bildirim türü.");
    }
}

// Fonksiyon Bildirimini Analiz Et
void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclAST* funcDecl) {
    std::cout << "Analyzing Function: " << funcDecl->name->name << std::endl; // Debug çıktısı

    // Fonksiyon ismini sembol tablosuna ekle (Eğer ilk geçişte eklenmediyse, veya burada özellikleri tamamla)
     SymbolInfo* funcSymbol = symbolTable.lookupCurrentScope(funcDecl->name->name).get();
     if (!funcSymbol) { // İlk geçişte eklenmemişse burada ekle (Basit yaklaşım)
         funcSymbol = new SymbolInfo(funcDecl->name->name, nullptr, funcDecl, false); // Tipini sonra belirle
         symbolTable.insert(funcDecl->name->name, std::shared_ptr<SymbolInfo>(funcSymbol));
     }

    // Fonksiyon kapsamına gir
    symbolTable.enterScope();

    // Fonksiyon parametrelerini sembol tablosuna ekle ve tiplerini analiz et
    // currentFunctionReturnType'ı ayarla
    // Fonksiyon gövdesini analiz et (BlockStatementAST)
     analyzeBlockStatement(funcDecl->body.get());

    // Fonksiyon kapsamından çık
    symbolTable.exitScope();

    // currentFunctionReturnType'ı sıfırla
     currentFunctionReturnType = nullptr;
}

// Struct Bildirimini Analiz Et
void SemanticAnalyzer::analyzeStructDecl(StructDeclAST* structDecl) {
    // Struct ismini sembol tablosuna ekle ve Tip Sistemine kaydet
    // Struct alanlarını analiz et (isimleri ve tipleri)
    // Struct alanları için yeni bir kapsam oluşturmak gerekebilir veya sadece sembol tablosunda işaretlenebilir.
}

// Enum Bildirimini Analiz Et
void SemanticAnalyzer::analyzeEnumDecl(EnumDeclAST* enumDecl) {
     // Enum ismini sembol tablosuna ekle ve Tip Sistemine kaydet
    // Enum varyantlarını sembol tablosuna ekle (enum'ın alt öğeleri olarak)
}

// Değişken Bildirimini Analiz Et (Global veya Yerel)
void SemanticAnalyzer::analyzeVarDecl(VarDeclAST* varDecl, bool isGlobal) {
    std::cout << "Analyzing Variable Declaration: " << varDecl->name->name << std::endl; // Debug çıktısı

    // Değişken isminin mevcut kapsamda daha önce tanımlanmadığını kontrol et
    if (symbolTable.lookupCurrentScope(varDecl->name->name)) {
         diagnostics.reportError(varDecl->location.filename, varDecl->location.line, varDecl->location.column,
                                 "'" + varDecl->name->name + "' ismi bu kapsamda zaten tanımlı.");
         // Hata durumunda sembol tablosuna eklemeyebiliriz
         return;
    }

    // Tip belirtilmişse, tip AST düğümünü analiz edip semantik tipe dönüştür
    Type* varType = nullptr;
    if (varDecl->type) {
        varType = analyzeTypeAST(varDecl->type.get());
        if (!varType || varType->id == Type::ERROR_TYPE) {
            // Tip analizi başarısız oldu, hata zaten raporlandı
            return;
        }
    }

    // Başlangıç değeri (initializer) varsa, ifadeyi analiz et ve tipini al
    Type* initializerType = nullptr;
    if (varDecl->initializer) {
        initializerType = analyzeExpression(varDecl->initializer.get());
        if (!initializerType || initializerType->id == Type::ERROR_TYPE) {
            // İfade analizi başarısız oldu, hata zaten raporlandı
             // initializerType'ı ERROR_TYPE olarak ayarla
        } else {
            // Eğer değişken tipi belirtilmemişse, başlangıç değerinin tipini kullan (Tür çıkarımı - Type Inference)
            if (!varType) {
                 varType = initializerType;
            } else {
                 // Değişken tipi belirtilmişse, başlangıç değerinin tipinin uyumlu olup olmadığını kontrol et (Atanabilirlik)
                 if (!typeSystem.isAssignable(initializerType, varType, varDecl->isMutable)) {
                     diagnostics.reportError(varDecl->initializer->location.filename, varDecl->initializer->location.line, varDecl->initializer->location.column,
                                             "'" + initializerType->toString() + "' tipi '" + varType->toString() + "' tipine atanamaz.");
                     // varType'ı ERROR_TYPE olarak işaretleyebiliriz
                     varType = typeSystem.getErrorType();
                 }
            }
        }
    } else {
        // Başlangıç değeri yok ve tip belirtilmemişse hata (CNT'de zorunlu olabilir?)
        if (!varType) {
             diagnostics.reportError(varDecl->location.filename, varDecl->location.line, varDecl->location.column,
                                     "Değişken '" + varDecl->name->name + "' başlangıç değeri veya tip belirtilmeden tanımlanamaz.");
             varType = typeSystem.getErrorType();
        }
    }

    // Eğer hala bir tipimiz yoksa veya hata tipi ise geri dön
     if (!varType || varType->id == Type::ERROR_TYPE) {
        // Hata zaten raporlandı, sembol tablosuna ekleme veya ERROR_TYPE ile ekleme yapılabilir.
         // Sembol tablosuna ERROR_TYPE ile ekleyip devam etmek diğer hataları bulmak için daha iyi olabilir.
         varType = typeSystem.getErrorType();
     }

    // Değişken bilgisini sembol tablosuna ekle
    auto symbol = std::make_shared<SymbolInfo>(varDecl->name->name, varType, varDecl, varDecl->isMutable);
    symbolTable.insert(varDecl->name->name, symbol);

    // Değişkenin AST düğümüne çözümlenmiş sembol bilgisini veya tipini ekle (Annotation/Decoration)
     varDecl->resolvedSymbol = symbol.get(); // Eğer VarDeclAST'te böyle bir üye varsa

    // Sahiplik Kuralları: Yeni tanımlanan değişken başlangıçta değere sahip olur (OWNED)
     ownershipChecker.recordVariableDeclaration(varDecl, varType, varDecl->isMutable, varDecl->location); // OwnershipChecker'a bildir
}


// Deyim analizcisi (Türüne göre dallanır)
void SemanticAnalyzer::analyzeStatement(StatementAST* stmt) {
     //std::cout << "Analyzing Statement: " << stmt->getNodeType() << std::endl; // Debug çıktısı
    if (BlockStatementAST* block = dynamic_cast<BlockStatementAST*>(stmt)) {
        analyzeBlockStatement(block);
    } else if (ExpressionStatementAST* exprStmt = dynamic_cast<ExpressionStatementAST*>(stmt)) {
        // İfadeyi analiz et, dönüş değerini yoksay (çünkü deyim olarak kullanılıyor)
        analyzeExpression(exprStmt->expression.get());
        // Sahiplik Kuralları: İfade sonucunda oluşan geçici değer ne olacak?
        // ownershipChecker.handleTemporaryValue(exprType, exprStmt->location);
    } else if (ImportStatementAST* importStmt = dynamic_cast<ImportStatementAST*>(stmt)) {
        analyzeImportStatement(importStmt);
    } else if (ReturnStatementAST* returnStmt = dynamic_cast<ReturnStatementAST*>(stmt)) {
        analyzeReturnStatement(returnStmt);
    } else if (BreakStatementAST* breakStmt = dynamic_cast<BreakStatementAST*>(stmt)) {
        analyzeBreakStatement(breakStmt);
    } else if (ContinueStatementAST* continueStmt = dynamic_cast<ContinueStatementAST*>(stmt)) {
        analyzeContinueStatement(continueStmt);
    } else if (WhileStatementAST* whileStmt = dynamic_cast<WhileStatementAST*>(stmt)) {
        analyzeWhileStatement(whileStmt);
    }
    // IfStatementAST veya ForStatementAST gibi diğer deyim türleri
    // VarDeclAST hem Declaration hem Statement olduğu için burada da yakalanabilir:
    else if (VarDeclAST* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
         // Yerel değişken bildirimi olarak analiz et
         analyzeVarDecl(varDecl, false);
    }
     else {
        diagnostics.reportError(stmt->location.filename, stmt->location.line, stmt->location.column,
                                "Anlamsal analiz için bilinmeyen deyim türü.");
    }
}

// Blok Deyimi Analiz Et
void SemanticAnalyzer::analyzeBlockStatement(BlockStatementAST* block) {
    // Yeni bir kapsam oluştur ve gir
    symbolTable.enterScope();

    // Blok içindeki deyimleri analiz et
    for (const auto& stmt_ptr : block->statements) {
        analyzeStatement(stmt_ptr.get());
    }

    // Kapsamdan çık
    symbolTable.exitScope();

    // Sahiplik Kuralları: Kapsam dışına çıkan değişkenlerin sahipliğini sonlandır
     ownershipChecker.endScope(symbolTable.getCurrentScope()); // OwnershipChecker'a bildir
}

// Import Deyimi Analiz Et
void SemanticAnalyzer::analyzeImportStatement(ImportStatementAST* importStmt) {
    // Modül çözümleyiciyi kullanarak import'u çözümle
    // Çözümlenen modül arayüzünden sembolleri alıp mevcut kapsama veya ilgili kapsama ekle
    // moduleResolver.resolveImport(importStmt, symbolTable.getCurrentScope());
}

// İfade analizcisi (Türüne göre dallanır ve ifade tipini döndürür)
Type* SemanticAnalyzer::analyzeExpression(ExpressionAST* expr) {
     if (!expr) return typeSystem.getErrorType();

    std::cout << "Analyzing Expression: " << expr->getNodeType() << std::endl; // Debug çıktısı

    Type* exprType = nullptr; // İfadenin semantik tipi

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
        exprType = analyzeIdentifier(id);
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
    // ... Diğer ifade türleri
    else {
        diagnostics.reportError(expr->location.filename, expr->location.line, expr->location.column,
                                "Anlamsal analiz için bilinmeyen ifade türü.");
        exprType = typeSystem.getErrorType();
    }

    // İfade AST düğümüne belirlenen semantik tipi ekle (Annotation/Decoration)
     expr->resolvedType = exprType; // Eğer ExpressionAST'te böyle bir üye varsa

    return exprType;
}

// Identifier Analiz Et (İsim Çözümleme)
Type* SemanticAnalyzer::analyzeIdentifier(IdentifierAST* identifier) {
    // İsimi sembol tablosunda ara (mevcut ve üst kapsamlarda)
    auto symbol = symbolTable.lookup(identifier->name);

    if (!symbol) {
        diagnostics.reportError(identifier->location.filename, identifier->location.line, identifier->location.column,
                                "'" + identifier->name + "' ismi tanımlı değil.");
        return typeSystem.getErrorType(); // Hata tipi döndür
    }

    // AST düğümüne çözümlenen sembol bilgisini ekle (Annotation)
     identifier->resolvedSymbol = symbol.get(); // Eğer IdentifierAST'te böyle bir üye varsa

    // Sembolün tipini döndür
    return symbol->type;
}

// Binary Operatör İfade Analiz Et (+, -, ==, && vb.)
Type* SemanticAnalyzer::analyzeBinaryOp(BinaryOpAST* binaryOp) {
    // Sol ve sağ operand ifadeleri analiz et ve tiplerini al
    Type* leftType = analyzeExpression(binaryOp->left.get());
    Type* rightType = analyzeExpression(binaryOp->right.get());

    // Eğer operand tipleri hata tipi ise, sonuç da hata tipidir.
    if (leftType->id == Type::ERROR_TYPE || rightType->id == Type::ERROR_TYPE) {
        return typeSystem.getErrorType();
    }

    // Operatöre göre tip uyumluluğunu ve sonuç tipini belirle
    Type* resultType = typeSystem.getErrorType(); // Varsayılan olarak hata tipi

    switch (binaryOp->op) {
        case Token::TOK_PLUS:
        case Token::TOK_MINUS:
        case Token::TOK_STAR:
        case Token::TOK_SLASH:
        case Token::TOK_PERCENT:
            // Aritmetik operatörler: Genellikle sayısal tiplerde tanımlıdır.
            // Operand tipleri sayısal mı? Uyumlu mu? Ortak bir tipe yükseltilmeli mi?
            if (typeSystem.areTypesEqual(leftType, rightType) && (leftType->id == Type::INT_TYPE || leftType->id == Type::FLOAT_TYPE)) {
                 resultType = leftType; // Sonuç tipi operandlarla aynı
            } else {
                 diagnostics.reportError(binaryOp->location.filename, binaryOp->location.line, binaryOp->location.column,
                                         "'" + binaryOp->lexeme + "' operatörü için uyumsuz operand tipleri: " + leftType->toString() + " ve " + rightType->toString());
            }
            break;
        case Token::TOK_EQ:
        case Token::TOK_NE:
        case Token::TOK_LT:
        case Token::TOK_GT:
        case Token::TOK_LE:
        case Token::TOK_GE:
            // Karşılaştırma operatörleri: Genellikle sayısal veya string gibi karşılaştırılabilir tiplerde. Sonuç bool.
            if (typeSystem.areTypesEqual(leftType, rightType) && (leftType->id == Type::INT_TYPE || leftType->id == Type::FLOAT_TYPE || leftType->id == Type::STRING_TYPE)) { // String karşılaştırması ekleyebilirsiniz
                 resultType = typeSystem.getBoolType();
            } else {
                 diagnostics.reportError(binaryOp->location.filename, binaryOp->location.line, binaryOp->location.column,
                                         "'" + binaryOp->lexeme + "' operatörü için uyumsuz operand tipleri: " + leftType->toString() + " ve " + rightType->toString());
            }
            break;
        case Token::TOK_AND:
        case Token::TOK_OR:
            // Mantıksal operatörler: Sadece boolean tiplerde tanımlıdır. Sonuç bool.
            if (leftType->id == Type::BOOL_TYPE && rightType->id == Type::BOOL_TYPE) {
                resultType = typeSystem.getBoolType();
            } else {
                diagnostics.reportError(binaryOp->location.filename, binaryOp->location.line, binaryOp->location.column,
                                        "'" + binaryOp->lexeme + "' operatörü sadece boolean tipleri için geçerlidir. Bulunan: " + leftType->toString() + " ve " + rightType->toString());
            }
            break;
        // ... Diğer ikili operatörler (bitişik atamalar += vb.)

        default:
            // Bilinmeyen ikili operatör
             diagnostics.reportError(binaryOp->location.filename, binaryOp->location.line, binaryOp->location.column,
                                    "Bilinmeyen ikili operatör anlamsal analizde.");
            break;
    }

    // BinaryOpAST düğümüne sonuç tipini ekle (Annotation)
     binaryOp->resolvedType = resultType;

    return resultType; // Sonuç tipini döndür
}


// Unary Operatör İfade Analiz Et (!, -, &, &mut, *)
Type* SemanticAnalyzer::analyzeUnaryOp(UnaryOpAST* unaryOp) {
    // Operand ifadeyi analiz et ve tipini al
    Type* operandType = analyzeExpression(unaryOp->operand.get());

    if (operandType->id == Type::ERROR_TYPE) {
        return typeSystem.getErrorType(); // Operand hata tipi ise sonuç da hata
    }

    Type* resultType = typeSystem.getErrorType(); // Varsayılan hata tipi

    switch (unaryOp->op) {
        case Token::TOK_NOT:
            // Mantıksal DEĞİL: Sadece boolean tipiyle çalışır. Sonuç bool.
            if (operandType->id == Type::BOOL_TYPE) {
                resultType = typeSystem.getBoolType();
            } else {
                 diagnostics.reportError(unaryOp->location.filename, unaryOp->location.line, unaryOp->location.column,
                                        "Mantıksal DEĞİL (!) operatörü sadece boolean tipleri için geçerlidir. Bulunan: " + operandType->toString());
            }
            break;
        case Token::TOK_MINUS:
            // Negatif işareti (-): Sayısal tiplerle çalışır. Sonuç aynı tip.
             if (operandType->id == Type::INT_TYPE || operandType->id == Type::FLOAT_TYPE) {
                resultType = operandType;
            } else {
                 diagnostics.reportError(unaryOp->location.filename, unaryOp->location.line, unaryOp->location.column,
                                        "Negatif (-) operatörü sadece sayısal tipler için geçerlidir. Bulunan: " + operandType->toString());
            }
            break;
        case Token::TOK_AND: // & (Immutable Referans)
            // Referans operatörü: Herhangi bir T tipi için &T referansı oluşturur.
            resultType = typeSystem.getReferenceType(operandType, false); // false: mutable değil

            // Sahiplik Kuralları: Bu bir ödünç alma işlemidir. OwnershipChecker'a bildir.
            // ownershipChecker.checkBorrow(unaryOp); // Operandın durumu ve ödünç alma kuralları burada kontrol edilir
             ownershipChecker.recordBorrow(operandVarDecl, BorrowInfo{unaryOp, unaryOp->location, false}); // Ödünç alan bilgisi

            // Yaşam Süresi: Oluşan referansın yaşam süresi belirlenmeli/doğrulanmalı.
            // analyzeLifetimes(unaryOp); // UnaryOp'un yaşam süresini belirle veya doğrula

            break;
        case Token::TOK_MUT: // &mut (Mutable Referans)
            // Mutable Referans operatörü: Herhangi bir T tipi için &mut T referansı oluşturur.
            // Operandın değiştirilebilir (mutable) bir konum (l-value) olması gerekir.
            // analyzeExpression() 'dan dönen ifade düğümünün atanabilir (l-value) olup olmadığını kontrol edin.
            // Örneğin, bir literal'in &mut referansını alamazsınız.
             if (!operandNode->isAssignable) { // isAssignable özelliği AST düğümünde tutulabilir
                 diagnostics.reportError(... "Değiştirilemez değerin (&mut) referansı alınamaz.");
                 // resultType error kalır
             } else {
                 resultType = typeSystem.getReferenceType(operandType, true); // true: mutable

                 // Sahiplik Kuralları: Mutable ödünç alma. ownershipChecker'a bildir.
                  ownershipChecker.checkBorrow(unaryOp); // Mutable ödünç alma kuralları burada kontrol edilir (tek mutable borç kuralı vb.)
                  ownershipChecker.recordBorrow(operandVarDecl, BorrowInfo{unaryOp, unaryOp->location, true});

                  Yaşam Süresi: analyzeLifetimes(unaryOp);
             }
            break;
        case Token::TOK_STAR: // * (Dereference)
            // Dereference operatörü: Pointer veya referans tipinden değer tipine döner.
            // Operandın bir pointer veya referans tipi olması gerekir.
            if (operandType->id == Type::REFERENCE_TYPE) { // || operandType->id == Type::POINTER_TYPE
                ReferenceType* refType = static_cast<ReferenceType*>(operandType);
                resultType = refType->referencedType; // Referansın gösterdiği tipi döndür

                // Sahiplik Kuralları: Dereference ile ödünç alınan değere erişim yapılıyor.
                // Kullanımın ödünç alma kurallarına uygun olduğunu kontrol et.
                 ownershipChecker.checkBorrowValidity(referencedVarDecl, unaryOp->location); // Ödünç alınan değer hala geçerli mi?
            } else {
                 diagnostics.reportError(unaryOp->location.filename, unaryOp->location.line, unaryOp->location.column,
                                        "Dereference (*) operatörü sadece referans veya pointer tipleri için geçerlidir. Bulunan: " + operandType->toString());
            }
            break;

        default:
             diagnostics.reportError(unaryOp->location.filename, unaryOp->location.line, unaryOp->location.column,
                                    "Bilinmeyen tekli operatör anlamsal analizde.");
            break;
    }

    // UnaryOpAST düğümüne sonuç tipini ekle (Annotation)
     unaryOp->resolvedType = resultType;

    return resultType; // Sonuç tipini döndür
}


// Tip AST düğümlerini semantik tiplere dönüştüren analizci
Type* SemanticAnalyzer::analyzeTypeAST(TypeAST* typeNode) {
    if (!typeNode) return typeSystem.getErrorType();

    Type* semanticType = nullptr;

    if (BaseTypeAST* baseType = dynamic_cast<BaseTypeAST*>(typeNode)) {
        // Temel tipi çözümle: "int", "float" gibi ana tipler veya kullanıcı tanımlı (struct, enum) isimleri
        if (baseType->name == "int") semanticType = typeSystem.getIntType();
        else if (baseType->name == "float") semanticType = typeSystem.getFloatType();
        else if (baseType->name == "bool") semanticType = typeSystem.getBoolType();
        else if (baseType->name == "string") semanticType = typeSystem.getStringType();
        else if (baseType->name == "char") semanticType = typeSystem.getCharType();
        // ... diğer temel tipler

        else {
            // Kullanıcı tanımlı tip (Struct veya Enum adı olmalı)
            // İsimi sembol tablosunda ara. Bir Struct veya Enum bildirimi olmalı.
            auto symbol = symbolTable.lookup(baseType->name);
            if (symbol && symbol->type && (symbol->type->id == Type::STRUCT_TYPE || symbol->type->id == Type::ENUM_TYPE)) {
                semanticType = symbol->type; // Çözümlenmiş struct/enum tipini kullan
                 baseType->resolvedSymbol = symbol.get(); // AST düğümüne sembol bilgisi ekle
            } else {
                diagnostics.reportError(baseType->location.filename, baseType->location.line, baseType->location.column,
                                        "'" + baseType->name + "' tanımlı bir tip ismi değil.");
                semanticType = typeSystem.getErrorType();
            }
        }
    } else if (ReferenceTypeAST* refType = dynamic_cast<ReferenceTypeAST*>(typeNode)) {
        semanticType = analyzeReferenceTypeAST(refType);
    } else if (ArrayTypeAST* arrayType = dynamic_cast<ArrayTypeAST*>(typeNode)) {
        semanticType = analyzeArrayTypeAST(arrayType);
    }
    // ... Diğer TypeAST türleri

    else {
         diagnostics.reportError(typeNode->location.filename, typeNode->location.line, typeNode->location.column,
                                "Anlamsal analiz için bilinmeyen tip AST düğümü türü.");
         semanticType = typeSystem.getErrorType();
    }

    // TypeAST düğümüne belirlenen semantik tipi ekle (Annotation)
     typeNode->resolvedSemanticType = semanticType; // Eğer TypeAST'te böyle bir üye varsa

    return semanticType; // Belirlenen semantik tipi döndür
}

// Reference Type AST Analiz Et (&T, &mut T)
Type* SemanticAnalyzer::analyzeReferenceTypeAST(ReferenceTypeAST* refTypeNode) {
    // Referansın gösterdiği tipi analiz et
    Type* referencedSemanticType = analyzeTypeAST(refTypeNode->referencedType.get());

    if (!referencedSemanticType || referencedSemanticType->id == Type::ERROR_TYPE) {
        return typeSystem.getErrorType(); // İç tip hata ise sonuç da hata
    }

    // Semantik ReferenceType objesini Tip Sisteminden al veya oluştur
    return typeSystem.getReferenceType(referencedSemanticType, refTypeNode->isMutable);
}

// Array Type AST Analiz Et ([T])
Type* SemanticAnalyzer::analyzeArrayTypeAST(ArrayTypeAST* arrayTypeNode) {
     // Eleman tipini analiz et
    Type* elementSemanticType = analyzeTypeAST(arrayTypeNode->elementType.get());

    if (!elementSemanticType || elementSemanticType->id == Type::ERROR_TYPE) {
        return typeSystem.getErrorType(); // Eleman tipi hata ise sonuç da hata
    }

    // Semantik ArrayType objesini Tip Sisteminden al veya oluştur
    return typeSystem.getArrayType(elementSemanticType);
}


// =======================================================================
// Anlamsal Kontrol ve Doğrulama Yardımcı Metodları Implementasyonu
// =======================================================================

// İki semantik tipin eşit olup olmadığını kontrol et
bool SemanticAnalyzer::areTypesEqual(Type* t1, Type* t2) {
    if (!t1 || !t2) return false;
    if (t1 == t2) return true; // Aynı pointer ise eşitler
    if (t1->id == Type::ERROR_TYPE || t2->id == Type::ERROR_TYPE) return false; // Hata tipleri eşit kabul edilmez

    return t1->isEqualTo(t2); // Tip sınıfının kendi isEqualTo metodunu kullan
}

// Bir tipin diğerine atanabilir olup olmadığını kontrol et (Mutability dikkate alınır)
bool SemanticAnalyzer::isAssignable(Type* valueType, Type* targetType, bool isTargetMutable) {
     if (!valueType || !targetType) return false;
     if (valueType->id == Type::ERROR_TYPE || targetType->id == Type::ERROR_TYPE) return false;

    return valueType->isAssignableTo(targetType, isTargetMutable); // Value tipinin kendi isAssignableTo metodunu kullan
}

// Fonksiyon çağrısının doğru olup olmadığını kontrol et (argüman sayısı/tipi)
void SemanticAnalyzer::checkFunctionCall(CallExpressionAST* callExpr, FunctionDeclAST* targetFunc) {
    // Argüman sayısını kontrol et
    if (callExpr->arguments.size() != targetFunc->arguments.size()) {
        diagnostics.reportError(callExpr->location.filename, callExpr->location.line, callExpr->location.column,
                                "Fonksiyon '" + targetFunc->name->name + "' için yanlış sayıda argüman. Beklenen: " + std::to_string(targetFunc->arguments.size()) + ", Bulunan: " + std::to_string(callExpr->arguments.size()));
        // Hata sonrası tip kontrolüne devam etmeyebiliriz
        return;
    }

    // Her argümanın tipini kontrol et
    for (size_t i = 0; i < callExpr->arguments.size(); ++i) {
        Type* argValueType = analyzeExpression(callExpr->arguments[i].get()); // Argüman ifadesinin tipini analiz et
        Type* paramType = targetFunc->arguments[i]->type.get(); // Parametre tipi AST düğümünden semantik tipi almalısınız.
                                                                // Muhtemelen FunctionArgAST->type zaten analyzeTypeAST'ten geçmiş olmalı.
        Type* paramSemanticType = analyzeTypeAST(targetFunc->arguments[i]->type.get()); // Veya burada analiz et

        if (!argValueType || !paramType) { // || !paramSemanticType
            // Hata zaten raporlandı
            continue;
        }

        // Argüman tipinin parametre tipine atanabilir olup olmadığını kontrol et
        // Parametrenin mutable olup olmadığını da kontrol etmeniz gerekebilir (eğer parametreler mutable olabiliyorsa)
        bool isParamMutable = targetFunc->arguments[i]->isMutable; // FunctionArgAST'te isMutable üyesi varsa
        if (!isAssignable(argValueType, paramType, isParamMutable)) { // isAssignable(argValueType, paramSemanticType, ...)
             diagnostics.reportError(callExpr->arguments[i]->location.filename, callExpr->arguments[i]->location.column, callExpr->arguments[i]->location.column,
                                     "Argüman " + std::to_string(i+1) + " için uyumsuz tip. Beklenen: " + paramType->toString() + ", Bulunan: " + argValueType->toString());
        }

        // Sahiplik Kuralları: Fonksiyon çağrısı argümanların sahipliğini taşıyabilir veya ödünç alabilir.
        // Bu kuralları OwnershipChecker'a bildirmeniz gerekir.
         ownershipChecker.handleFunctionArgument(callExpr->arguments[i].get(), targetFunc->arguments[i].get());
    }
}
