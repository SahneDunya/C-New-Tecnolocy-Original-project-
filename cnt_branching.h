#ifndef CNT_BRANCHING_H
#define CNT_BRANCHING_H

#include <stdbool.h>
#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"   // Return ifadesi için (ExpressionNode)

// Dallanma İfade Türleri (isteğe bağlı)
typedef enum {
    BRANCH_BREAK,
    BRANCH_CONTINUE,
    BRANCH_RETURN
} BranchingStatementType;

// Break İfadesi AST Düğümü
typedef struct BreakStatementNode {
    ASTNode base;
} BreakStatementNode;

// Continue İfadesi AST Düğümü
typedef struct ContinueStatementNode {
    ASTNode base;
} ContinueStatementNode;

// Return İfadesi AST Düğümü
typedef struct ReturnStatementNode {
    ASTNode base;
    ExpressionNode* value; // Döndürülecek değer (isteğe bağlı)
} ReturnStatementNode;

// Yeni bir Break ifadesi AST düğümü oluşturma fonksiyonu
BreakStatementNode* break_statement_create();

// Yeni bir Continue ifadesi AST düğümü oluşturma fonksiyonu
ContinueStatementNode* continue_statement_create();

// Yeni bir Return ifadesi AST düğümü oluşturma fonksiyonu
ReturnStatementNode* return_statement_create(ExpressionNode* value);

#endif