#ifndef CNT_COMPILER_STATEMENTS_H
#define CNT_COMPILER_STATEMENTS_H

#include "ast.h" // Temel AST sınıfları için
#include "expressions.h" // Deyimler ifadeleri içerebilir
#include "types.h" // Tip bilgisi (varsa)


#include <vector>
#include <memory> // std::unique_ptr için

// Blok Deyimi ({ ... })
struct BlockStatementAST : public StatementAST {
    std::vector<std::unique_ptr<StatementAST>> statements; // Blok içindeki deyimler

    // Scope bilgisi semantik analizde tutulabilir (SymbolTable'da). AST düğümünde pointer tutmak isteyebilirsiniz.
     Scope* associatedScope = nullptr;

    BlockStatementAST() = default;
    BlockStatementAST(TokenLocation loc) { location = loc; }

    std::string getNodeType() const override { return "BlockStatementAST"; }

    void addStatement(std::unique_ptr<StatementAST> stmt) {
        statements.push_back(std::move(stmt));
    }
};

// İfade Deyimi (expression ;)
struct ExpressionStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> expression; // Noktalı virgülle biten ifade

    // resolvedSemanticType zaten ExpressionAST'ten miras alındığı için burada otomatik var.
    // Ama bir deyimin tipi genellikle void'dir. expression->resolvedSemanticType'ı kullanmak daha doğru.


    ExpressionStatementAST(std::unique_ptr<ExpressionAST> expr, TokenLocation loc)
        : expression(std::move(expr)) { location = loc; }

    std::string getNodeType() const override { return "ExpressionStatementAST"; }
};


// Import Deyimi (import path::to::module [as name];)
struct ImportStatementAST : public StatementAST {
    std::vector<std::string> path; // Modül yolu segmentleri (örn: {"std", "io"})
    std::string fullPathLexeme; // Orijinal "std::io" lexeme'i (hata raporlama için faydalı)
    std::unique_ptr<IdentifierAST> alias; // Alias (eğer kullanıldıysa)

    // Semantic Analiz sonuçları
    std::shared_ptr<ModuleInterface> resolvedInterface = nullptr; // Çözümlenmiş modül arayüzü

    ImportStatementAST(std::vector<std::string> p, std::string full_path, std::unique_ptr<IdentifierAST> a, TokenLocation loc)
        : path(std::move(p)), fullPathLexeme(std::move(full_path)), alias(std::move(a)) { location = loc; }


    std::string getNodeType() const override { return "ImportStatementAST"; }
};

// Return Deyimi (return expression?;)
struct ReturnStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> returnValue; // Dönüş değeri ifadesi (isteğe bağlı)

    // Semantic Analiz sonuçları
     returnValue->resolvedSemanticType'ı kullanın.


    ReturnStatementAST(std::unique_ptr<ExpressionAST> val, TokenLocation loc)
        : returnValue(std::move(val)) { location = loc; }

    std::string getNodeType() const override { return "ReturnStatementAST"; }
};

// Break Deyimi (break;)
struct BreakStatementAST : public StatementAST {
    // Belki bir etiket (label) içerebilir (Rust'taki gibi break 'label;)
     std::unique_ptr<IdentifierAST> label;

    // Semantic Analiz sonuçları
    // Break'in hangi döngüye ait olduğu bilgisi SEMA'da takip edilebilir veya AST düğümüne eklenebilir.
    // AST düğümüne pointer eklemek, CodeGen'de break hedef blokunu bulmak için faydalı olabilir.
     BlockStatementAST* targetLoopBlock = nullptr; // Hangi döngü bloğuna ait olduğunu işaret eden pointer

    BreakStatementAST(TokenLocation loc) { location = loc; }
    // BreakStatementAST(std::unique_ptr<IdentifierAST> lbl, TokenLocation loc) : label(std::move(lbl)) { location = loc; }

    std::string getNodeType() const override { return "BreakStatementAST"; }
};

// Continue Deyimi (continue;)
struct ContinueStatementAST : public StatementAST {
    // Belki bir etiket (label) içerebilir
     std::unique_ptr<IdentifierAST> label;

     // Semantic Analiz sonuçları
     BlockStatementAST* targetLoopBlock = nullptr; // Hangi döngü bloğuna ait olduğunu işaret eden pointer


    ContinueStatementAST(TokenLocation loc) { location = loc; }
     ContinueStatementAST(std::unique_ptr<IdentifierAST> lbl, TokenLocation loc) : label(std::move(lbl)) { location = loc; }


    std::string getNodeType() const override { return "ContinueStatementAST"; }
};

// While Döngüsü (while condition block)
struct WhileStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> condition; // Koşul ifadesi
    std::unique_ptr<BlockStatementAST> body;    // Döngü gövdesi

    // Semantic Analiz sonuçları
    // condition->resolvedSemanticType bool olmalı.
    // Döngü bloğunu CodeGenContext'te takip etmek yeterli olabilir, AST'ye pointer eklemeye gerek olmayabilir.

    WhileStatementAST(std::unique_ptr<ExpressionAST> cond, std::unique_ptr<BlockStatementAST> b, TokenLocation loc)
        : condition(std::move(cond)), body(std::move(b)) { location = loc; }

    std::string getNodeType() const override { return "WhileStatementAST"; }
};

// If Deyimi (Eğer match dışında if/else desteklemeye karar verirseniz)
 struct IfStatementAST : public StatementAST {
     std::unique_ptr<ExpressionAST> condition;
     std::unique_ptr<BlockStatementAST> thenBlock;
     std::unique_ptr<StatementAST> elseBranch; // BlockStatementAST veya başka bir IfStatementAST olabilir
 };

// For Döngüsü
 struct ForStatementAST : public StatementAST {
     std::unique_ptr<PatternAST> pattern; // Döngü değişkeni
     std::unique_ptr<ExpressionAST> iterator; // Üzerinde yinelenecek ifade
     std::unique_ptr<BlockStatementAST> body;
 };


#endif // CNT_COMPILER_STATEMENTS_H
