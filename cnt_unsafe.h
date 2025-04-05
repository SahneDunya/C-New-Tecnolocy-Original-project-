#ifndef CNT_UNSAFE_H
#define CNT_UNSAFE_H

#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_statement.h"    // Unsafe bloğu içindeki ifadeler için (StatementListNode)

// Unsafe Blok AST Düğümü
typedef struct UnsafeBlockNode {
    ASTNode base;
    StatementListNode* body; // Unsafe bloğu içindeki ifadeler
} UnsafeBlockNode;

// Yeni bir Unsafe Blok AST düğümü oluşturma fonksiyonu
UnsafeBlockNode* unsafe_block_create(StatementListNode* body);

#endif