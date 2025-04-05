#ifndef CNT_TRY_CATCH_H
#define CNT_TRY_CATCH_H

#include <stdbool.h>
#include "cnt_ast.h"       // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_statement.h"   // Try ve Catch blokları için (StatementListNode)
#include "cnt_type.h"        // Yakalanacak exception tipleri için (Type)

// Catch Bloğu AST Düğümü
typedef struct CatchClauseNode {
    ASTNode base;
    Type* exception_type; // Yakalanacak exception'ın tipi (NULL ise tüm tipleri yakalar)
    char* exception_variable_name;
    StatementListNode* body; // Catch bloğu içindeki ifadeler
    struct CatchClauseNode* next; // Bir sonraki catch bloğu için
} CatchClauseNode;

// Try İfadesi AST Düğümü
typedef struct TryStatementNode {
    ASTNode base;
    StatementListNode* try_block;   // try bloğu içindeki ifadeler
    CatchClauseNode* first_catch; // İlk catch bloğu
    StatementListNode* finally_block; // İsteğe bağlı: finally bloğu
} TryStatementNode;

// Yeni bir Catch Clause AST düğümü oluşturma fonksiyonu
CatchClauseNode* catch_clause_create(Type* exception_type, StatementListNode* body);

// Yeni bir Try ifadesi AST düğümü oluşturma fonksiyonu
TryStatementNode* try_statement_create(StatementListNode* try_block, CatchClauseNode* first_catch, StatementListNode* finally_block);

#endif