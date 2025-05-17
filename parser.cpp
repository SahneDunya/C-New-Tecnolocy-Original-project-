#include "parser.h"
#include <iostream> // Hata ayıklama çıktısı için

// Gerekli AST düğüm başlıklarını ekleyin (Eğer ast.h hepsini içermiyorsa)
// Örneğin:
 #include "ast_declarations.h"
 #include "ast_statements.h"
 #include "ast_expressions.h"
 #include "ast_types.h"


// Parser Kurucu
Parser::Parser(Lexer& lex) : lexer(lex) {
    // İlk iki token'ı alarak parser'ı başlat
    currentToken = lexer.getNextToken();
    peekToken = lexer.getNextToken();
}

// currentToken'ı tüketir ve peekToken'ı currentToken yapar, lexer'dan yeni token alır
void Parser::consume() {
    currentToken = std::move(peekToken); // currentToken'a taşı
    peekToken = lexer.getNextToken();    // Lexer'dan yeni bir token al
    // İsteğe bağlı: Token tüketildiğine dair debug çıktısı
     std::cout << "Consumed token: " << currentToken.getTypeString() << " ('" << currentToken.lexeme << "') at "
               << currentToken.filename << ":" << currentToken.line << ":" << currentToken.column << std::endl;
}

// currentToken belirli bir türde mi? Tüketmez.
bool Parser::check(Token::Type type) {
    return currentToken.type == type;
}

// peekToken belirli bir türde mi? Tüketmez.
bool Parser::checkNext(Token::Type type) {
    return peekToken.type == type;
}


// currentToken belirli bir türde mi? Eğer evetse tüketir ve true döner.
bool Parser::match(Token::Type type) {
    if (!check(type)) {
        return false;
    }
    consume();
    return true;
}

// currentToken'ın belirli bir türde olmasını bekler, evetse tüketir, değilse hata raporlar.
void Parser::expect(Token::Type type, const std::string& errorMessage) {
    if (!match(type)) {
        reportErrorAtCurrentToken(errorMessage + ". Beklenen: " + Token(type, 0, 0, "").getTypeString() + ", Bulunan: " + currentToken.getTypeString());
        // Hata kurtarma mekanizması burada tetiklenebilir
         synchronize(); // Eğer hata kurtarma implemente edildiyse
    }
}

// Hata raporlama (Diagnostics sistemini kullanır)
void Parser::reportParsingError(const std::string& message) {
     ReportError(currentToken.filename, currentToken.line, currentToken.column, message);
}

void Parser::reportErrorAtCurrentToken(const std::string& message) {
     ReportError(currentToken.filename, currentToken.line, currentToken.column, message);
}


// =======================================================================
// Gramer Kurallarına Karşılık Gelen Ayrıştırma Metodları Implementasyonu
// Burası dilinizin gramerini C++ koduna döktüğünüz yerdir.
// Her metot, beklenen sözdizimini okumalı, AST düğümleri oluşturmalı
// ve çocuk düğümleri için diğer parse metodlarını çağırmalıdır.
// =======================================================================

// Programın tamamını ayrıştır (En üst seviye)
std::unique_ptr<ProgramAST> Parser::parseProgram() {
    // Program kök düğümünü oluştur
    auto program = std::make_unique<ProgramAST>();
    program->location = currentToken.location; // Program konumu ilk token'ın konumu olabilir

    // Dosya sonuna (TOK_EOF) kadar bildirimleri (declaration) ayrıştır
    while (!check(Token::TOK_EOF)) {
        if (auto decl = parseDeclaration()) {
            program->addDeclaration(std::move(decl));
        } else {
            // Eğer bir bildirim ayrıştırılamazsa, bu bir hatadır.
            // reportParsingError fonksiyonu zaten çağrılmıştır.
            // Hata kurtarma mekanizması (eğer varsa) burada yardımcı olabilir.
            // Şimdilik, hatayı raporlayıp sonraki olası bildirime atlamaya çalışalım.
             if (check(Token::TOK_EOF)) break; // Sonsuz döngüyü önle
             consume(); // Hata token'ını veya yanlış token'ı atla
        }
    }

    // Dosya sonu token'ını tüket
    expect(Token::TOK_EOF, "Beklenmedik token dosya sonunda.");

    return program;
}

// Bildirimleri ayrıştır (Fonksiyon, Struct, Enum, Global Değişken)
std::unique_ptr<DeclarationAST> Parser::parseDeclaration() {
    // Hangi tür bildirimin geldiğini belirle
    if (check(Token::TOK_FN)) {
        return parseFunctionDeclaration();
    } else if (check(Token::TOK_STRUCT)) {
        return parseStructDeclaration();
    } else if (check(Token::TOK_ENUM)) {
        return parseEnumDeclaration();
    }
    // Not: Global değişken bildirimleri TOK_LET veya TOK_MUT ile başlayabilir
    // ve fonksiyon dışında bulunurlar. parseVariableDeclaration() metodunu
    // global değişkenler için de kullanabilirsiniz.
    else if (check(Token::TOK_LET) || check(Token::TOK_MUT)) {
         return parseVariableDeclaration(true); // true global olduğunu belirtir
    }
    // ... diğer bildirim türleri

    // Tanımsız bir bildirim başlangıcı
    reportErrorAtCurrentToken("Beklenmedik token: Bildirim bekleniyordu.");
    return nullptr; // Hata durumunda nullptr döndür
}

// Örnek: Basit bir fonksiyon bildirimi ayrıştırıcı (fn name() -> Type { ... })
std::unique_ptr<FunctionDeclAST> Parser::parseFunctionDeclaration() {
    // TOK_FN anahtar kelimesini bekleyip tüket
    expect(Token::TOK_FN, "'fn' anahtar kelimesi bekleniyor.");
    Token fnToken = currentToken; // fn kelimesinin konumu (AST düğümü için kullanışlı)
    consume(); // 'fn' tüket

    // Fonksiyon ismini ayrıştır (Tanımlayıcı bekleniyor)
    auto functionName = parseIdentifier();
    if (!functionName) {
         reportParsingError("Fonksiyon ismi bekleniyor.");
         return nullptr; // Hata
    }

    // Parametre listesini ayrıştır (Beklenen: '(' arguments? ')')
    expect(Token::TOK_LPAREN, "Fonksiyon ismi sonrası '(' bekleniyor.");
    // Argümanları ayrıştıracak bir metod çağrılabilir (parseFunctionArguments())
    // std::vector<std::unique_ptr<FunctionArgAST>> args = parseFunctionArguments();
    expect(Token::TOK_RPAREN, "Fonksiyon argüman listesi sonrası ')' bekleniyor.");

    // Dönüş türünü ayrıştır (İsteğe bağlı: '->' Type)
    std::unique_ptr<TypeAST> returnType = nullptr;
    if (match(Token::TOK_ARROW)) { // '->' varsa
        returnType = parseType(); // Dönüş türünü ayrıştır
        if (!returnType) {
            reportParsingError("Dönüş türü bekleniyor.");
            return nullptr; // Hata
        }
    }

    // Fonksiyon gövdesini ayrıştır (Blok deyimi bekleniyor '{ ... }')
    auto body = parseBlockStatement();
    if (!body) {
         reportParsingError("Fonksiyon gövdesi (blok) bekleniyor.");
         return nullptr; // Hata
    }

    // FunctionDeclAST düğümünü oluştur ve döndür
    auto funcDecl = std::make_unique<FunctionDeclAST>(); // FunctionDeclAST yapınız olmalı
    funcDecl->location = fnToken.location;
    funcDecl->name = std::move(functionName); // İsim AST düğümünü taşı
     funcDecl->arguments = std::move(args); // Argümanları taşı
    funcDecl->returnType = std::move(returnType); // Dönüş türünü taşı
    funcDecl->body = std::move(body); // Gövdeyi taşı

    return funcDecl;
}

// Örnek: Basit bir Block Statement ayrıştırıcı ({ ... })
std::unique_ptr<BlockStatementAST> Parser::parseBlockStatement() {
    expect(Token::TOK_LBRACE, "Blok başlangıcı '{' bekleniyor.");
    Token blockToken = currentToken;
    consume(); // '{' tüket

    auto block = std::make_unique<BlockStatementAST>(); // BlockStatementAST yapınız olmalı
    block->location = blockToken.location;

    // '}' karakterine veya dosya sonuna kadar deyimleri ayrıştır
    while (!check(Token::TOK_RBRACE) && !check(Token::TOK_EOF)) {
        if (auto stmt = parseStatement()) {
            block->addStatement(std::move(stmt)); // Blok içine deyimi ekle
        } else {
            // Hata oluştu, hata kurtarma (synchronize) veya atlama yapılabilir.
             if (check(Token::TOK_EOF) || check(Token::TOK_RBRACE)) break; // Sonsuz döngüyü önle
             consume(); // Hata token'ını atla
        }
    }

    expect(Token::TOK_RBRACE, "Blok sonu '}' bekleniyor.");
     consume() '}' karakterini expect() içinde tüketti

    return block;
}

// Örnek: Basit bir Expression Statement ayrıştırıcı (expression ;)
std::unique_ptr<StatementAST> Parser::parseStatement() {
    // CNT'de tüm ifadeler deyim olarak kullanılabilir ve sonunda noktalı virgül (;) gerektirir (Rust benzeri)
    // veya bir blok döndüren ifadeler noktalı virgül gerektirmez (if, match).
    // Bu implementasyon sadece 'expression ;' formatını varsayıyor.
    // Gerçek gramere göre burası dallanmalıdır.

    // Bir ifade ayrıştır
    auto expr = parseExpression();

    if (!expr) {
        // İfade ayrıştırılamadı, hata zaten raporlanmıştır.
        return nullptr;
    }

    // Noktalı virgülü bekle ve tüket
    expect(Token::TOK_SEMICOLON, "Deyim sonu ';' bekleniyor.");

    // ExpressionStatementAST düğümünü oluştur ve döndür
    auto exprStmt = std::make_unique<ExpressionStatementAST>(); // ExpressionStatementAST yapınız olmalı
    exprStmt->location = expr->location; // Konum ifade ile aynı olabilir
    exprStmt->expression = std::move(expr);

    return exprStmt;
}

// Örnek: Basit bir Atama İfadesi ayrıştırıcı (identifier = expression)
std::unique_ptr<ExpressionAST> Parser::parseAssignmentExpression() {
    // Önce daha yüksek öncelikli bir ifade ayrıştır (örn: Mantıksal VEYA)
    auto expr = parseLogicalOrExpression();

    // Eşittir (=) operatörü varsa
    if (match(Token::TOK_ASSIGN)) {
        // Sol tarafın atanabilir (l-value) olup olmadığını kontrol et (semantik analizde yapılır)
        // Şimdilik sadece AST'yi oluştur. Sol tarafın bir Identifier veya üye erişimi olması beklenir.
        // AST düğümünün türünü kontrol ederek atanabilirlik belirlenebilir.
        auto assignOpToken = currentToken; // = token'ının konumu
        consume(); // '=' tüket

        // Sağ taraf ifadeyi ayrıştır
        auto right = parseAssignmentExpression(); // Atama sağdan sola ilişkilidir, bu yüzden rekürsif çağrı

        if (!right) {
             reportParsingError("Atama ifadesinin sağ tarafı eksik.");
             return nullptr; // Hata
        }

        // Atama ifadesi AST düğümünü oluştur (AssignmentAST yapınız olmalı)
        auto assignmentExpr = std::make_unique<AssignmentAST>();
        assignmentExpr->location = assignOpToken.location;
        assignmentExpr->left = std::move(expr); // Sol tarafı ekle
        assignmentExpr->right = std::move(right); // Sağ tarafı ekle

        return assignmentExpr; // Atama ifadesini döndür

    }
    // Eğer eşittir operatörü yoksa, sadece ayrıştırdığımız ifadeyi döndür
    return expr;
}


// Örnek: Basit Mantıksal OR (||) İfadesi ayrıştırıcı (expr || expr)
std::unique_ptr<ExpressionAST> Parser::parseLogicalOrExpression() {
    // Önce daha yüksek öncelikli bir ifade ayrıştır (örn: Mantıksal VE)
    auto left = parseLogicalAndExpression();

    // '||' operatörleri olduğu sürece döngüye devam et
    while (match(Token::TOK_OR)) {
        Token opToken = currentToken; // Operatörün konumu
        consume(); // '||' tüket

        // Sağ operandı ayrıştır (aynı veya daha yüksek öncelikli)
        auto right = parseLogicalAndExpression();
         if (!right) {
             reportParsingError("Mantıksal OR ifadesinin sağ tarafı eksik.");
             return nullptr; // Hata
         }

        // Yeni bir BinaryOpAST düğümü oluştur (Sol operand olarak önceki sonuç, operatör, sağ operand)
        auto binaryOp = std::make_unique<BinaryOpAST>(Token::TOK_OR, std::move(left), std::move(right), opToken.location);
        left = std::move(binaryOp); // Yeni oluşturulan ifadeyi sol operand yap (zincirleme için)
    }

    // Zincirleme bittiğinde son ifadeyi döndür
    return left;
}

// Diğer Expression ayrıştırma metodları (parseLogicalAndExpression, parseEqualityExpression, vb.)
// benzer bir yapıya sahip olacaktır, sadece eşleşen operatör türleri ve çağrılan alt ayrıştırma metodu değişir.
// Operatör önceliği sırasına göre:
// parseLogicalAndExpression -> calls parseEqualityExpression
// parseEqualityExpression -> calls parseComparisonExpression (ops: ==, !=)
// parseComparisonExpression -> calls parseAdditiveExpression (ops: <, >, <=, >=)
// parseAdditiveExpression -> calls parseMultiplicativeExpression (ops: +, -)
// parseMultiplicativeExpression -> calls parseUnaryExpression (ops: *, /, %)
// parseUnaryExpression -> calls parseCallOrMemberAccessExpression (ops: !, -prefix, &, &mut, *)
// parseCallOrMemberAccessExpression -> calls parsePrimaryExpression (. , [])
// parsePrimaryExpression -> handles literals, identifiers, parenthesized expressions

// Örnek: Basit bir Primary Expression ayrıştırıcı (Literaller, Tanımlayıcılar, Parantezli İfadeler)
std::unique_ptr<ExpressionAST> Parser::parsePrimaryExpression() {
    Token token = currentToken; // Mevcut token'ı al

    switch (token.type) {
        case Token::TOK_IDENTIFIER: {
            consume(); // Tanımlayıcıyı tüket
            auto identifier = std::make_unique<IdentifierAST>(); // IdentifierAST yapınız olmalı
            identifier->location = token.location;
            identifier->name = std::move(token.lexeme);
            return identifier;
        }
        case Token::TOK_INT_LITERAL: {
            consume(); // Literal'i tüket
            // String değeri int'e dönüştür. Hata oluşursa raporla.
            try {
                 int val = std::stoi(token.lexeme);
                 return std::make_unique<IntLiteralAST>(val, token.location); // IntLiteralAST kullan
            } catch (const std::exception& e) {
                 reportErrorAtCurrentToken("Geçersiz tamsayı değişmezi: " + token.lexeme);
                 return nullptr; // Hata durumunda nullptr dön
            }
        }
        case Token::TOK_FLOAT_LITERAL: {
            consume(); // Literal'i tüket
             // String değeri float'a dönüştür. Hata oluşursa raporla.
            try {
                 float val = std::stof(token.lexeme); // veya double val = std::stod(token.lexeme);
                 auto floatLiteral = std::make_unique<FloatLiteralAST>(); // FloatLiteralAST yapınız olmalı
                 floatLiteral->location = token.location;
                 floatLiteral->value = val;
                 return floatLiteral;
            } catch (const std::exception& e) {
                 reportErrorAtCurrentToken("Geçersiz ondalıklı sayı değişmezi: " + token.lexeme);
                 return nullptr; // Hata durumunda nullptr dön
            }
        }
        case Token::TOK_STRING_LITERAL: {
            consume(); // Literal'i tüket
             auto stringLiteral = std::make_unique<StringLiteralAST>(); // StringLiteralAST yapınız olmalı
             stringLiteral->location = token.location;
             stringLiteral->value = std::move(token.lexeme); // Lexeme'i taşı
             return stringLiteral;
        }
        case Token::TOK_CHAR_LITERAL: {
            consume(); // Literal'i tüket
             auto charLiteral = std::make_unique<CharLiteralAST>(); // CharLiteralAST yapınız olmalı
             charLiteral->location = token.location;
             // Karakter değişmezi genellikle tek karakterdir (escape sequence hariç)
             // Lexer'da içeriği ayrıştırırken '\n' gibi tek karaktere dönüştürmüş olmalısınız.
             if (!token.lexeme.empty()) {
                 charLiteral->value = token.lexeme[0];
             } else {
                  // Lexer'dan boş karakter gelmişse (bu bir lexing hatası olmalıydı)
                  reportErrorAtCurrentToken("Boş karakter değişmezi.");
                  return nullptr;
             }
             return charLiteral;
        }
        case Token::TOK_TRUE: {
            consume(); // true tüket
             auto boolLiteral = std::make_unique<BoolLiteralAST>(); // BoolLiteralAST yapınız olmalı
             boolLiteral->location = token.location;
             boolLiteral->value = true;
             return boolLiteral;
        }
        case Token::TOK_FALSE: {
             consume(); // false tüket
             auto boolLiteral = std::make_unique<BoolLiteralAST>(); // BoolLiteralAST yapınız olmalı
             boolLiteral->location = token.location;
             boolLiteral->value = false;
             return boolLiteral;
        }
        case Token::TOK_LPAREN: { // Parantezli ifade ( ( expression ) )
            consume(); // '(' tüket
            auto expr = parseExpression(); // Parantez içindeki ifadeyi ayrıştır
            if (!expr) {
                reportParsingError("Parantezli ifade içinde eksik ifade.");
                return nullptr; // Hata
            }
            expect(Token::TOK_RPAREN, "Parantezli ifade kapatılmamış. ')' bekleniyor.");
            return expr; // Parantez içindeki ifadenin kendisini döndür
        }
        // ... diğer primary expression türleri (Array literal [1, 2], Tuple literal (1, "a") vb.)

        default:
            // Beklenmedik token
            reportErrorAtCurrentToken("Beklenmedik token: Birincil ifade (literal, tanımlayıcı veya parantezli ifade) bekleniyordu.");
            // Hata kurtarma mekanizması burada faydalı olur.
             synchronize();
            return nullptr; // Hata durumunda nullptr döndür
    }
}

// Ana ayrıştırma metodunu başlat
std::unique_ptr<ProgramAST> Parser::parse() {
    // Programı ayrıştırmaya başla
    return parseProgram();
}

// =======================================================================
// Hata Kurtarma (İsteğe bağlı ama önerilen)
// Bir parser hatası oluştuğunda, derleyicinin mümkünse derlemeye devam etmesini sağlar.
// Sözdizimsel hataların çoğunu tek bir derleme çalışmasında görebilirsiniz.
// =======================================================================

 void Parser::synchronize() {
     consume(); // Hata token'ını tüket

     while (!check(Token::TOK_EOF)) {
//         // Önceki token bir senkronizasyon noktası mıydı?
         switch (currentToken.type) {
             case Token::TOK_SEMICOLON: // Noktalı virgül
             case Token::TOK_RBRACE:    // Kapanış küme parantezi }
             case Token::TOK_FN:        // Yeni bir fonksiyon bildirimi
             case Token::TOK_STRUCT:    // Yeni bir struct bildirimi
             case Token::TOK_ENUM:      // Yeni bir enum bildirimi
             case Token::TOK_LET:       // Yeni bir let bildirimi
             case Token::TOK_IMPORT:    // Yeni bir import bildirimi
             case Token::TOK_RETURN:    // Yeni bir return deyimi
//             // ... diğer potansiyel senkronizasyon noktaları
                 return; // Bir senkronizasyon noktası bulduk, buradan devam edebiliriz
             default:
                 // Devam et
                 ;
         }

//         // Sonraki token bir senkronizasyon noktası mı?
         switch (peekToken.type) {
             case Token::TOK_SEMICOLON:
             case Token::TOK_RBRACE:
             case Token::TOK_FN:
             case Token::TOK_STRUCT:
             case Token::TOK_ENUM:
             case Token::TOK_LET:
             case Token::TOK_IMPORT:
             case Token::TOK_RETURN:
//             // ...
                 consume(); // Senkronizasyon token'ını tüket ve fonksiyondan çık
                 return;
             default:
                 // Devam et
                 ;
         }

         consume(); // Senkronizasyon noktası bulamazsak tokenları atlamaya devam et
     }
 }
