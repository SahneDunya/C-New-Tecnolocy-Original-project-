#ifndef CNT_IF_STATEMENT_H
#define CNT_IF_STATEMENT_H

#include <stdbool.h>
#include "cnt_ast.h"       // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"  // Koşul ifadeleri için (Expression)
#include "cnt_statement.h"   // İfade blokları için (StatementList)

// Koşullu İfade Türleri
typedef enum {
    IF_STATEMENT,
    ELSE_IF_STATEMENT,
    ELSE_STATEMENT
} IfStatementType;

// If İfadesi AST Düğümü
typedef struct IfStatementNode {
    ASTNode base;
    ExpressionNode* condition;
    StatementListNode* body;
    struct IfStatementNode* next_else_if; // Zincirleme else if ifadeleri için
    struct IfStatementNode* else_branch;   // Son else ifadesi için
} IfStatementNode;

// Else If İfadesi AST Düğümü (IfStatementNode yapısını kullanabilir)

// Else İfadesi AST Düğümü
typedef struct ElseStatementNode {
    ASTNode base;
    StatementListNode* body;
} ElseStatementNode;

// Yeni bir If ifadesi AST düğümü oluşturma fonksiyonu
IfStatementNode* if_statement_create(ExpressionNode* condition, StatementListNode* body);

// Yeni bir Else If ifadesi AST düğümü oluşturma fonksiyonu
IfStatementNode* else_if_statement_create(ExpressionNode* condition, StatementListNode* body);

// Yeni bir Else ifadesi AST düğümü oluşturma fonksiyonu
ElseStatementNode* else_statement_create(StatementListNode* body);

#endif