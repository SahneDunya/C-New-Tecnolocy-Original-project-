#ifndef CNT_LOOP_H
#define CNT_LOOP_H

#include <stdbool.h>
#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"   // Döngü koşulları ve ifadeleri için (ExpressionNode)
#include "cnt_statement.h"    // Döngü gövdeleri için (StatementListNode)

// Döngü Türleri (isteğe bağlı)
typedef enum {
    LOOP_FOR,
    LOOP_WHILE,
    LOOP_DO_WHILE, // İstenirse eklenebilir
    LOOP_RANGE_BASED_FOR // Modern özellik
} LoopType;

// For Döngüsü AST Düğümü
typedef struct ForLoopNode {
    ASTNode base;
    ExpressionNode* initialization; // Döngü başlangıç ifadesi (örneğin, i = 0)
    ExpressionNode* condition;    // Döngü devam koşulu (örneğin, i < 10)
    ExpressionNode* increment;    // Döngü artırım/azaltım ifadesi (örneğin, i++)
    StatementListNode* body;       // Döngü gövdesi
} ForLoopNode;

// While Döngüsü AST Düğümü
typedef struct WhileLoopNode {
    ASTNode base;
    ExpressionNode* condition;    // Döngü devam koşulu
    StatementListNode* body;       // Döngü gövdesi
} WhileLoopNode;

// Yeni bir For döngüsü AST düğümü oluşturma fonksiyonu
ForLoopNode* for_loop_create(ExpressionNode* initialization, ExpressionNode* condition, ExpressionNode* increment, StatementListNode* body);

// Yeni bir While döngüsü AST düğümü oluşturma fonksiyonu
WhileLoopNode* while_loop_create(ExpressionNode* condition, StatementListNode* body);

    typedef struct RangeBasedForLoopNode {
    ASTNode base;
    IdentifierNode* variable;    // Döngü değişkeni
    ExpressionNode* iterable;    // Üzerinde dönelecek ifade (dizi, koleksiyon vb.)
    StatementListNode* body;       // Döngü gövdesi
} RangeBasedForLoopNode;

 RangeBasedForLoopNode* range_based_for_loop_create(IdentifierNode* variable, ExpressionNode* iterable, StatementListNode* body);

#endif