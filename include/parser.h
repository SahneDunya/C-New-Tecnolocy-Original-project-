#ifndef CNT_COMPILER_PARSER_H
#define CNT_COMPILER_PARSER_H

#include "lexer.h"       // Lexer sınıfı için
#include "token.h"       // Token yapısı için
#include "ast.h"         // AST düğüm tanımları için (Temel sınıflar)
#include "expressions.h" // İfade AST düğümleri
#include "statements.h"  // Deyim AST düğümleri
#include "declarations.h"// Bildirim AST düğümleri (isPublic bayrağını içermeli)
#include "types.h"       // Tip AST düğümleri
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

    // Yardımcı metodlar
    void consume();
    bool match(Token::Type type);
    bool check(Token::Type type);
    bool checkNext(Token::Type type);
    void expect(Token::Type type, const std::string& errorMessage);

    // Hata raporlama
    void reportParsingError(const std::string& message);
    void reportErrorAtCurrentToken(const std::string& message);


    // =======================================================================
    // Gramer Kurallarına Karşılık Gelen Ayrıştırma Metodları
    // =======================================================================

    // Programın tamamını ayrıştır
    std::unique_ptr<ProgramAST> parseProgram();

    // Bildirimleri ayrıştır (pub anahtar kelimesini burada kontrol edeceğiz)
    // Bu metod pub'ı tüketir ve ilgili alt metoda isPublic bayrağını geçirir.
    std::unique_ptr<DeclarationAST> parseDeclaration();

    // Belirli bildirim türlerini ayrıştır (isPublic bayrağını alırlar)
    std::unique_ptr<FunctionDeclAST> parseFunctionDeclaration(bool isPublic);
    std::unique_ptr<StructDeclAST> parseStructDeclaration(bool isPublic);
    std::unique_ptr<EnumDeclAST> parseEnumDeclaration(bool isPublic);
    // Global değişken bildirimleri için (let/mut ile başlayanlar)
    std::unique_ptr<VarDeclAST> parseVariableDeclaration(bool isPublic, bool isGlobal = false);


    // Deyimleri ayrıştır
    std::unique_ptr<StatementAST> parseStatement();
    std::unique_ptr<BlockStatementAST> parseBlockStatement();
    std::unique_ptr<ImportStatementAST> parseImportStatement();
    std::unique_ptr<ReturnStatementAST> parseReturnStatement();
    std::unique_ptr<BreakStatementAST> parseBreakStatement();
    std::unique_ptr<ContinueStatementAST> parseContinueStatement();
     // Diğer deyimler...
    std::unique_ptr<WhileStatementAST> parseWhileStatement(); // While döngüsü örneği

    // İfadeleri ayrıştır (Operatör önceliğine göre zincir)
    std::unique_ptr<ExpressionAST> parseExpression();
    std::unique_ptr<ExpressionAST> parseAssignmentExpression(); // =
    std::unique_ptr<ExpressionAST> parseLogicalOrExpression();  // ||
    std::unique_ptr<ExpressionAST> parseLogicalAndExpression(); // &&
    std::unique_ptr<ExpressionAST> parseEqualityExpression();   // ==, !=
    std::unique_ptr<ExpressionAST> parseComparisonExpression(); // <, >, <=, >=
    std::unique_ptr<ExpressionAST> parseAdditiveExpression();   // +, -
    std::unique_ptr<ExpressionAST> parseMultiplicativeExpression(); // *, /, %
    std::unique_ptr<ExpressionAST> parseUnaryExpression();      // !, -, &, &mut, *
    std::unique_ptr<ExpressionAST> parseCallOrMemberAccessExpression(); // func(), obj.member, array[index]
    std::unique_ptr<ExpressionAST> parsePrimaryExpression();    // Literals, identifiers, parenthesized expressions

    // Match ifadesi için özel ayrıştırma
    std::unique_ptr<MatchExpressionAST> parseMatchExpression();
    std::unique_ptr<MatchArmAST> parseMatchArm();

    // Tip ifadelerini ayrıştır
    std::unique_ptr<TypeAST> parseType();
    std::unique_ptr<TypeAST> parseBaseType();
    std::unique_ptr<TypeAST> parseReferenceType(bool isMutable);
    std::unique_ptr<TypeAST> parseArrayType(std::unique_ptr<TypeAST> elementType);

    // Diğer yardımcı ayrıştırma metodları
    std::vector<std::unique_ptr<StatementAST>> parseBlockContent();
    std::vector<std::unique_ptr<FunctionArgAST>> parseFunctionArguments();
    std::vector<std::unique_ptr<StructFieldAST>> parseStructFields();
    std::unique_ptr<IdentifierAST> parseIdentifier(); // Sadece tanımlayıcıyı ayrıştırır


    // Hata kurtarma (Opsiyonel)
     void synchronize();


public:
    // Kurucu: Lexer ve Diagnostics referanslarını alır
    Parser(Lexer& lex, Diagnostics& diag);

    // Ana ayrıştırma metodunu başlat
    std::unique_ptr<ProgramAST> parse();
};

#endif // CNT_COMPILER_PARSER_H
