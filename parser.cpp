#include "parser.h"
#include <iostream> // Hata ayıklama çıktısı için

// AST düğüm implementasyonlarının başlıkları (Eğer ayrı dosyalardaysa)
 #include "ast_declarations.h"
 #include "ast_statements.h"
 #include "ast_expressions.h"
 #include "ast_types.h"


// Parser Kurucu
Parser::Parser(Lexer& lex, Diagnostics& diag) : lexer(lex), diagnostics(diag) {
    // İlk iki token'ı alarak parser'ı başlat
    currentToken = lexer.getNextToken();
    peekToken = lexer.getNextToken();
}

// ... Yardımcı metodlar (consume, match, check, checkNext, expect, reportParsingError, reportErrorAtCurrentToken)
// Bu metodlar önceki parser.cpp kodundan buraya kopyalanacak.


// Programın tamamını ayrıştır
std::unique_ptr<ProgramAST> Parser::parseProgram() {
    auto program = std::make_unique<ProgramAST>();
    program->location = currentToken.location;

    while (!check(Token::TOK_EOF)) {
        // Bir bildirim ayrıştır
        if (auto decl = parseDeclaration()) {
            program->addDeclaration(std::move(decl));
        } else {
             // Hata oluştu, hata zaten parseDeclaration içinde raporlandı.
             // Hata kurtarma (synchronize) burada çağrılabilir.
              synchronize(); // Eğer implemente edildi ise
             if (check(Token::TOK_EOF)) break; // Sonsuz döngüyü önle
             consume(); // Hata token'ını veya yanlış token'ı atla
        }
    }

    expect(Token::TOK_EOF, "Dosya sonu bekleniyor.");

    return program;
}

// Bildirimleri ayrıştır (pub anahtar kelimesini burada kontrol ediyoruz)
std::unique_ptr<DeclarationAST> Parser::parseDeclaration() {
    bool isPublic = false;
    // Eğer bir sonraki token 'pub' ise
    if (check(Token::TOK_PUB)) {
        isPublic = true;
        consume(); // 'pub' token'ını tüket

        // 'pub' sonrası geçerli bir bildirim türü (fn, struct, enum, let, mut) bekliyoruz.
        // Eğer gelmezse hata raporla.
        if (!check(Token::TOK_FN) && !check(Token::TOK_STRUCT) && !check(Token::TOK_ENUM) &&
            !check(Token::TOK_LET) && !check(Token::TOK_MUT)) {
             reportErrorAtCurrentToken("'pub' anahtar kelimesi sonrası fonksiyon, struct, enum veya değişken bildirimi bekleniyor.");
             // Hata kurtarma mekanizması burada tetiklenebilir.
              synchronize();
             return nullptr; // Hata durumunda nullptr döndür
        }
    }

    // 'pub' sonrası gelen bildirimin türünü belirle ve ilgili ayrıştırıcıyı çağır
    if (check(Token::TOK_FN)) {
        return parseFunctionDeclaration(isPublic);
    } else if (check(Token::TOK_STRUCT)) {
        return parseStructDeclaration(isPublic);
    } else if (check(Token::TOK_ENUM)) {
        return parseEnumDeclaration(isPublic);
    } else if (check(Token::TOK_LET) || check(Token::TOK_MUT)) {
         // Global değişken bildirimi olarak ayrıştır
         return parseVariableDeclaration(isPublic, true); // true global olduğunu belirtir
    }
    // ... Diğer bildirim türleri (örn: trait, impl)

    // 'pub' olmayan veya 'pub' sonrası tanımsız bir token
    if (!isPublic) { // Eğer 'pub' token'ı hiç gelmediyse
         reportErrorAtCurrentToken("Beklenmedik token: Bildirim (fn, struct, enum, let, mut) bekleniyor.");
    }
    // Hata kurtarma mekanizması burada tetiklenebilir.
     synchronize();

    return nullptr; // Hata durumunda nullptr döndür
}

// Örnek: Basit bir fonksiyon bildirimi ayrıştırıcı (fn name() -> Type { ... })
// isPublic bayrağını alacak şekilde güncellendi.
std::unique_ptr<FunctionDeclAST> Parser::parseFunctionDeclaration(bool isPublic) {
    // TOK_FN anahtar kelimesini bekle ve tüket
    // 'pub' token'ı parseDeclaration() içinde tüketildiği için burada kontrol etmeye gerek yok.
    expect(Token::TOK_FN, "'fn' anahtar kelimesi bekleniyor.");
    Token fnToken = currentToken; // fn kelimesinin konumu
    consume(); // 'fn' tüket

    // Fonksiyon ismini ayrıştır
    auto functionName = parseIdentifier();
    if (!functionName) {
         reportParsingError("Fonksiyon ismi bekleniyor.");
         return nullptr;
    }

    // Parametre listesini ayrıştır ('(' args? ')')
    expect(Token::TOK_LPAREN, "Fonksiyon ismi sonrası '(' bekleniyor.");
    // Argümanları ayrıştıracak metod çağrılabilir
     std::vector<std::unique_ptr<FunctionArgAST>> args = parseFunctionArguments();
    expect(Token::TOK_RPAREN, "Fonksiyon argüman listesi sonrası ')' bekleniyor.");

    // Dönüş türünü ayrıştır (İsteğe bağlı '->' Type)
    std::unique_ptr<TypeAST> returnType = nullptr;
    if (match(Token::TOK_ARROW)) {
        returnType = parseType();
        if (!returnType) {
            reportParsingError("Dönüş türü bekleniyor.");
            return nullptr;
        }
    }

    // Fonksiyon gövdesini ayrıştır (Blok deyimi '{ ... }')
    auto body = parseBlockStatement();
    if (!body) {
         reportParsingError("Fonksiyon gövdesi (blok) bekleniyor.");
         return nullptr;
    }

    // FunctionDeclAST düğümünü oluştur, isPublic bayrağını ayarla ve döndür
    auto funcDecl = std::make_unique<FunctionDeclAST>(); // FunctionDeclAST yapınız olmalı ve isPublic üyesi olmalı
    funcDecl->location = fnToken.location;
    funcDecl->name = std::move(functionName);
    // funcDecl->arguments = std::move(args);
    funcDecl->returnType = std::move(returnType);
    funcDecl->body = std::move(body);
    funcDecl->isPublic = isPublic; // <-- isPublic bayrağını ayarla

    return funcDecl;
}

// Örnek: Basit bir Block Statement ayrıştırıcı ({ ... })
// Bu metod isPublic almaz çünkü blokların kendisi public/private olmaz, içindekiler olur (Scope).
std::unique_ptr<BlockStatementAST> Parser::parseBlockStatement() {
    expect(Token::TOK_LBRACE, "Blok başlangıcı '{' bekleniyor.");
    Token blockToken = currentToken;
    consume(); // '{' tüket

    auto block = std::make_unique<BlockStatementAST>();
    block->location = blockToken.location;

    while (!check(Token::TOK_RBRACE) && !check(Token::TOK_EOF)) {
        if (auto stmt = parseStatement()) {
            block->addStatement(std::move(stmt));
        } else {
             // Hata oluştu, hata zaten raporlandı.
              synchronize();
             if (check(Token::TOK_EOF) || check(Token::TOK_RBRACE)) break;
             consume();
        }
    }

    expect(Token::TOK_RBRACE, "Blok sonu '}' bekleniyor.");

    return block;
}

// Örnek: Basit bir Expression Statement ayrıştırıcı (expression ;)
// Bu metod isPublic almaz, çünkü deyimlerin kendisi public/private olmaz.
std::unique_ptr<StatementAST> Parser::parseStatement() {
    // CNT'de bir ifade bir deyim olabilir (sonunda noktalı virgül ile)
    // veya if/match gibi ifadeler blok döndürüyorsa noktalı virgül gerekmeyebilir.
    // Bu implementasyon, sadece "expression ;" formatını varsayıyor.
    // Diğer deyim türleri için burada dallanma eklenmelidir (if, while, for, return, break, continue, import, let/mut local).

     // Import deyimi?
    if (check(Token::TOK_IMPORT)) {
        return parseImportStatement();
    }
    // Return deyimi?
    else if (check(Token::TOK_RETURN)) {
        return parseReturnStatement();
    }
    // Break deyimi?
    else if (check(Token::TOK_BREAK)) {
        return parseBreakStatement();
    }
    // Continue deyimi?
    else if (check(Token::TOK_CONTINUE)) {
        return parseContinueStatement();
    }
    // While döngüsü?
    else if (check(Token::TOK_WHILE)) {
         return parseWhileStatement();
    }
    // Yerel değişken bildirimi?
    else if (check(Token::TOK_LET) || check(Token::TOK_MUT)) {
         // Yerel değişken bildirimi olarak ayrıştır (isPublic false)
         return parseVariableDeclaration(false, false); // false public değil, false global değil
    }
    // Blok deyimi?
    else if (check(Token::TOK_LBRACE)) {
        return parseBlockStatement();
    }
    // ... Diğer deyim türleri

    // Eğer yukarıdakilerden hiçbiri değilse, bunun bir İfade Deyimi olduğunu varsayalım.
    // Bir ifade ayrıştır.
    auto expr = parseExpression();

    if (!expr) {
        // İfade ayrıştırılamadı, hata zaten raporlandı.
        return nullptr; // Hata durumunda nullptr döndür
    }

    // Noktalı virgülü bekle ve tüket.
    expect(Token::TOK_SEMICOLON, "Deyim sonu ';' bekleniyor.");

    // ExpressionStatementAST düğümünü oluştur ve döndür
    auto exprStmt = std::make_unique<ExpressionStatementAST>(std::move(expr), currentToken.location); // Konum ';' token'ının konumu olabilir

    return exprStmt;
}


// Örnek: Basit bir Değişken Bildirimi ayrıştırıcı (let/mut name: Type = value;)
// isPublic ve isGlobal bayraklarını alacak şekilde güncellendi.
std::unique_ptr<VarDeclAST> Parser::parseVariableDeclaration(bool isPublic, bool isGlobal) {
    TokenKind mutabilityToken = check(Token::TOK_MUT) ? Token::TOK_MUT : Token::TOK_LET;
    bool isMutable = (mutabilityToken == Token::TOK_MUT);

    Token varDeclToken = currentToken; // let/mut token'ının konumu

    // 'let' veya 'mut' anahtar kelimesini bekle ve tüket
    if (isMutable) expect(Token::TOK_MUT, "'mut' anahtar kelimesi bekleniyor.");
    else expect(Token::TOK_LET, "'let' anahtar kelimesi bekleniyor.");

    // Değişken ismini ayrıştır
    auto variableName = parseIdentifier();
    if (!variableName) {
         reportParsingError("Değişken ismi bekleniyor.");
         return nullptr;
    }

    // Tip belirtimi ayrıştır (İsteğe bağlı ': Type')
    std::unique_ptr<TypeAST> variableType = nullptr;
    if (match(Token::TOK_COLON)) { // ':' varsa
        variableType = parseType(); // Tipi ayrıştır
        if (!variableType) {
            reportParsingError("Değişken tipi bekleniyor.");
            return nullptr;
        }
    }

    // Başlangıç değeri ayrıştır (İsteğe bağlı '= value')
    std::unique_ptr<ExpressionAST> initializerValue = nullptr;
    if (match(Token::TOK_ASSIGN)) { // '=' varsa
        initializerValue = parseExpression(); // Başlangıç değeri ifadesini ayrıştır
        if (!initializerValue) {
            reportParsingError("Değişken başlangıç değeri bekleniyor.");
            return nullptr;
        }
    } else {
         // Eğer başlangıç değeri yoksa ve tip de belirtilmemişse (CNT'nin kuralına göre)
         if (!variableType) {
             reportErrorAtCurrentToken("Değişken '" + variableName->name + "' başlangıç değeri veya tip belirtilmeden tanımlanamaz.");
             // Hata sonrası ne yapılacağı parser politikasına bağlı.
             // Hata kurtarma mekanizması çağrılabilir.
              synchronize();
             return nullptr;
         }
    }

    // Global değişken bildirimi ise noktalı virgül (Rust gibi) gerektirebilir.
    // Yerel değişken bildirimi parseStatement içinde ele alınıyorsa, noktalı virgül
    // parseStatement tarafından zaten beklenir.
     if (isGlobal) {
        expect(Token::TOK_SEMICOLON, "Global değişken bildirimi sonu ';' bekleniyor.");
     }


    // VarDeclAST düğümünü oluştur, isPublic ve isGlobal bayraklarını ayarla
    auto varDecl = std::make_unique<VarDeclAST>(
        std::move(variableName),
        std::move(variableType),
        std::move(initializerValue),
        isMutable,
        varDeclToken.location // let/mut token'ının konumu
    );
    varDecl->isPublic = isPublic; // <-- isPublic bayrağını ayarla

    return varDecl;
}


// ... Diğer parse metodları (parseExpression, parseBinaryOp, parsePrimaryExpression,
// parseType, parseFunctionArguments, parseStructFields, parseIdentifier vb.)
// Bu metodlar önceki parser.cpp kodundan buraya kopyalanacak ve isPublic bayrağını almayacaklar
// veya kendi alt çağrılarına isPublic bayrağını aktarmayacaklar çünkü
// pub sadece bildirimin başına gelir, ifadenin veya tipin başına değil.


// Ana ayrıştırma metodunu başlat
std::unique_ptr<ProgramAST> Parser::parse() {
    // Programı ayrıştırmaya başla
    return parseProgram();
}

// ... Hata kurtarma (synchronize) metodu implementasyonu (Opsiyonel)
