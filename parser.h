#ifndef CNT_COMPILER_PARSER_H
#define CNT_COMPILER_PARSER_H

#include "lexer.h"       // Lexer sınıfı için
#include "token.h"       // Token yapısı için
#include "ast.h"         // AST düğüm tanımları için
#include "diagnostics.h" // Hata raporlama için

#include <memory>        // std::unique_ptr için
#include <vector>
#include <string>


// Parser sınıfı
class Parser {
private:
    Lexer& lexer;        // Kullanılacak Lexer referansı
    Token currentToken;  // Şu anda işlenmekte olan token
    Token peekToken;     // Bir sonraki token (ileriye bakma)

    // Token manipülasyonu ve kontrolü için yardımcı metodlar
    void consume(); // currentToken'ı tüketir ve peekToken'ı currentToken yapar, lexer'dan yeni token alır
    void advance(); // consume() ile aynı işi yapar, farklı isimle kullanışlı olabilir. consume'u tercih edelim.

    bool match(Token::Type type); // currentToken belirli bir türde mi? Eğer evetse tüketir ve true döner.
    bool check(Token::Type type); // currentToken belirli bir türde mi? Tüketmez.
    bool checkNext(Token::Type type); // peekToken belirli bir türde mi? Tüketmez.

    void expect(Token::Type type, const std::string& errorMessage); // currentToken'ın belirli bir türde olmasını bekler, evetse tüketir, değilse hata raporlar.

    // Hata raporlama
    void reportParsingError(const std::string& message);
    void reportErrorAtCurrentToken(const std::string& message); // Mevcut token konumunda hata bildir

    // =======================================================================
    // Gramer Kurallarına Karşılık Gelen Ayrıştırma Metodları
    // Dilinizin gramerini (EBNF gibi) buradaki metodlara haritalayın.
    // Her metod genellikle bir AST düğümü döndürür.
    // =======================================================================

    // Programın tamamını ayrıştır (En üst seviye)
    std::unique_ptr<ProgramAST> parseProgram();

    // Bildirimleri ayrıştır (Fonksiyonlar, structlar, enumlar, global değişkenler)
    std::unique_ptr<DeclarationAST> parseDeclaration();
    std::unique_ptr<FunctionDeclAST> parseFunctionDeclaration(); // TOK_FN anahtar kelimesi ile başlar
    std::unique_ptr<StructDeclAST> parseStructDeclaration();   // TOK_STRUCT anahtar kelimesi ile başlar
    std::unique_ptr<EnumDeclAST> parseEnumDeclaration();     // TOK_ENUM anahtar kelimesi ile başlar
    std::unique_ptr<VarDeclAST> parseVariableDeclaration(bool isGlobal = false); // TOK_LET veya TOK_MUT ile başlar

    // Deyimleri ayrıştır (Statements)
    std::unique_ptr<StatementAST> parseStatement();
    std::unique_ptr<BlockStatementAST> parseBlockStatement(); // { ... } Bloğu
    std::unique_ptr<ImportStatementAST> parseImportStatement(); // TOK_IMPORT ile başlar
    std::unique_ptr<ReturnStatementAST> parseReturnStatement(); // TOK_RETURN ile başlar
    std::unique_ptr<BreakStatementAST> parseBreakStatement();   // TOK_BREAK ile başlar
    std::unique_ptr<ContinueStatementAST> parseContinueStatement(); // TOK_CONTINUE ile başlar
    // Not: match kontrol yapısı burada StatementAST veya ExpressionAST olarak ele alınabilir.
    // Genellikle match bir ifade olduğu için ExpressionAST içinde tanımlanabilir ve parseExpression() çağrı zincirinde yer alabilir.
    // Ancak bağımsız bir deyim olarak da kullanılabilir. Dilinizin gramerine bağlı. Match'i Expression olarak ele alalım şimdilik.
     std::unique_ptr<IfStatementAST> parseIfStatement(); // Eğer if/else desteklemeye karar verirseniz

    // İfadeleri ayrıştır (Expressions) - Operatör önceliğine göre ayrıştırma zinciri
    // Önce en düşük öncelikli operatörlerden başlanır (atama) ve giderek yüksek öncelikli olanlara inilir.
    std::unique_ptr<ExpressionAST> parseExpression(); // Başlangıç noktası
    std::unique_ptr<ExpressionAST> parseAssignmentExpression(); // =
    std::unique_ptr<ExpressionAST> parseLogicalOrExpression();  // ||
    std::unique_ptr<ExpressionAST> parseLogicalAndExpression(); // &&
    std::unique_ptr<ExpressionAST> parseEqualityExpression();   // ==, !=
    std::unique_ptr<ExpressionAST> parseComparisonExpression(); // <, >, <=, >=
    std::unique_ptr<ExpressionAST> parseAdditiveExpression();   // +, -
    std::unique_ptr<ExpressionAST> parseMultiplicativeExpression(); // *, /, %
    std::unique_ptr<ExpressionAST> parseUnaryExpression();      // !, - (prefix), & (referans), &mut (mutable referans), * (dereference)
    std::unique_ptr<ExpressionAST> parseCallOrMemberAccessExpression(); // Fonksiyon çağrısı, üye erişimi (.), array erişimi ([])
    std::unique_ptr<ExpressionAST> parsePrimaryExpression();    // Değişmezler (literals), tanımlayıcılar (identifiers), parantezli ifadeler

    // Match ifadesi için özel ayrıştırma
    std::unique_ptr<MatchExpressionAST> parseMatchExpression(); // TOK_MATCH ile başlar
    std::unique_ptr<MatchArmAST> parseMatchArm(); // pattern => expression,

    // Tip ifadelerini ayrıştır (int, string, MyStruct, &mut i32 gibi)
    std::unique_ptr<TypeAST> parseType();
    // Yardımcı tipler
    std::unique_ptr<TypeAST> parseBaseType(); // int, float, bool, string, identifier
    std::unique_ptr<TypeAST> parseReferenceType(bool isMutable); // & veya &mut
    std::unique_ptr<TypeAST> parseArrayType(std::unique_ptr<TypeAST> elementType); // [] ile

    // Diğer yardımcı ayrıştırma metodları
    std::vector<std::unique_ptr<StatementAST>> parseBlockContent(); // Bir blok içindeki deyimleri ayrıştırır
    std::vector<std::unique_ptr<FunctionArgAST>> parseFunctionArguments(); // Fonksiyon tanımlarındaki argümanları ayrıştırır
    std::vector<std::unique_ptr<StructFieldAST>> parseStructFields(); // Struct tanımlarındaki alanları ayrıştırır
    std::unique_ptr<IdentifierAST> parseIdentifier(); // TOK_IDENTIFIER bekler ve IdentityAST döndürür


    // Hata kurtarma (Gelişmiş parserlar için)
    // Parsing hatası oluştuğunda derleyicinin tamamen durmasını engellemek için
    // senkronizasyon noktalarına (noktalı virgül, küme parantezi sonu gibi) kadar atlama mantığı.
     void synchronize();


public:
    // Kurucu: Lexer referansını alır
    Parser(Lexer& lex);

    // Ana ayrıştırma metodunu başlat
    std::unique_ptr<ProgramAST> parse();
};

#endif // CNT_COMPILER_PARSER_H
