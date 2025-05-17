#ifndef CNT_COMPILER_STATEMENTS_H
#define CNT_COMPILER_STATEMENTS_H

#include "ast.h" // Temel AST sınıfları için
#include "expressions.h" // Deyimler ifadeleri içerebilir

#include <vector>
#include <memory> // std::unique_ptr için

// Blok Deyimi ({ ... })
struct BlockStatementAST : public StatementAST {
    std::vector<std::unique_ptr<StatementAST>> statements; // Blok içindeki deyimler

    BlockStatementAST() = default; // Varsayılan kurucu
    BlockStatementAST(TokenLocation loc) { location = loc; }

    std::string getNodeType() const override { return "BlockStatementAST"; }

    void addStatement(std::unique_ptr<StatementAST> stmt) {
        statements.push_back(std::move(stmt));
    }
};

// İfade Deyimi (expression ;)
struct ExpressionStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> expression; // Noktalı virgülle biten ifade

    ExpressionStatementAST(std::unique_ptr<ExpressionAST> expr, TokenLocation loc)
        : expression(std::move(expr)) { location = loc; }

    std::string getNodeType() const override { return "ExpressionStatementAST"; }
};


// Import Deyimi (import path::to::module [as name];)
struct ImportStatementAST : public StatementAST {
    std::vector<std::string> path; // Modül yolu (örn: ["std", "io"])
    // İsteğe bağlı olarak 'as' ile verilen yeni isim
    std::unique_ptr<IdentifierAST> alias; // Alias (eğer kullanıldıysa)

    ImportStatementAST(std::vector<std::string> p, std::unique_ptr<IdentifierAST> a, TokenLocation loc)
        : path(std::move(p)), alias(std::move(a)) { location = loc; }
     // Yol vector'ünü string olarak almak ve sonra parse etmek daha esnek olabilir.
     ImportStatementAST(std::string full_path_lexeme, std::unique_ptr<IdentifierAST> a, TokenLocation loc)
         : alias(std::move(a)) {
             location = loc;
             // full_path_lexeme stringini "::" ayıracına göre parse edip 'path' vectorüne doldurmalısınız.
             // Örnek basit ayrıştırma (Gerçek implementasyonda daha dikkatli olunmalı):
             size_t start = 0;
             size_t end = full_path_lexeme.find("::");
             while (end != std::string::npos) {
                 path.push_back(full_path_lexeme.substr(start, end - start));
                 start = end + 2; // "::" uzunluğu
                 end = full_path_lexeme.find("::", start);
             }
             path.push_back(full_path_lexeme.substr(start)); // Son parçayı ekle
         }


    std::string getNodeType() const override { return "ImportStatementAST"; }
};

// Return Deyimi (return expression?;)
struct ReturnStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> returnValue; // Dönüş değeri (isteğe bağlı)

    ReturnStatementAST(std::unique_ptr<ExpressionAST> val, TokenLocation loc)
        : returnValue(std::move(val)) { location = loc; }

    std::string getNodeType() const override { return "ReturnStatementAST"; }
};

// Break Deyimi (break;)
struct BreakStatementAST : public StatementAST {
    // Belki bir etiket (label) içerebilir (Rust'taki gibi break 'label;)
    // std::string label;
    BreakStatementAST(TokenLocation loc) { location = loc; }
    std::string getNodeType() const override { return "BreakStatementAST"; }
};

// Continue Deyimi (continue;)
struct ContinueStatementAST : public StatementAST {
    // Belki bir etiket (label) içerebilir
    // std::string label;
    ContinueStatementAST(TokenLocation loc) { location = loc; }
    std::string getNodeType() const override { return "ContinueStatementAST"; }
};

// If Deyimi (Eğer match dışında if/else desteklemeye karar verirseniz)
 struct IfStatementAST : public StatementAST {
     std::unique_ptr<ExpressionAST> condition;
     std::unique_ptr<BlockStatementAST> thenBlock;
     std::unique_ptr<StatementAST> elseBranch; // BlockStatementAST veya başka bir IfStatementAST olabilir
//
//     IfStatementAST(std::unique_ptr<ExpressionAST> cond, std::unique_ptr<BlockStatementAST> then_b, std::unique_ptr<StatementAST> else_b, TokenLocation loc)
         // condition(std::move(cond)), thenBlock(std::move(then_b)), elseBranch(std::move(else_b)) { location = loc; }
//
     std::string getNodeType() const override { return "IfStatementAST"; }
 };

// While Döngüsü
struct WhileStatementAST : public StatementAST {
    std::unique_ptr<ExpressionAST> condition;
    std::unique_ptr<BlockStatementAST> body;

    // WhileStatementAST(std::unique_ptr<ExpressionAST> cond, std::unique_ptr<BlockStatementAST> b, TokenLocation loc)
        // : condition(std::move(cond)), body(std::move(b)) { location = loc; }

    std::string getNodeType() const override { return "WhileStatementAST"; }
};

// For Döngüsü (Range tabanlı, Iterator tabanlı veya C benzeri olabilir. Rust'takine yakın tasarlayalım)
 struct ForStatementAST : public StatementAST {
     std::unique_ptr<PatternAST> pattern; // Döngü değişkeni (örn: 'i' veya 'x')
     std::unique_ptr<ExpressionAST> iterator; // Üzerinde yinelenecek ifade (örn: 0..10, my_vector)
     std::unique_ptr<BlockStatementAST> body;
//
//     ForStatementAST(std::unique_ptr<PatternAST> p, std::unique_ptr<ExpressionAST> iter, std::unique_ptr<BlockStatementAST> b, TokenLocation loc)
         : pattern(std::move(p)), iterator(std::move(iter)), body(std::move(b)) { location = loc; }

     std::string getNodeType() const override { return "ForStatementAST"; }
 };


#endif // CNT_COMPILER_STATEMENTS_H
