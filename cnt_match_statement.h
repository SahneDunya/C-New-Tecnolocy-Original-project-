#ifndef CNT_MATCH_STATEMENT_H
#define CNT_MATCH_STATEMENT_H

#include <stdbool.h>
#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"   // Match edilecek ifade ve desenler için (ExpressionNode)
#include "cnt_statement.h"    // Case blokları için (StatementListNode)

// Match Case AST Düğümü
typedef struct MatchCaseNode {
    ASTNode base;
    ExpressionNode* pattern;    // Eşleştirilecek desen (literal, değişken vb.)
    StatementListNode* body;       // Desen eşleşirse çalıştırılacak ifade bloğu
    struct MatchCaseNode* next; // Bir sonraki case için
} MatchCaseNode;

// Match İfadesi AST Düğümü
typedef struct MatchStatementNode {
    ASTNode base;
    ExpressionNode* expression; // Üzerinde eşleştirme yapılacak ifade
    MatchCaseNode* first_case;   // İlk case
    StatementListNode* default_case; // İsteğe bağlı: Hiçbir case eşleşmezse çalıştırılacak default blok
} MatchStatementNode;

// Yeni bir Match Case AST düğümü oluşturma fonksiyonu
MatchCaseNode* match_case_create(ExpressionNode* pattern, StatementListNode* body);

// Yeni bir Match ifadesi AST düğümü oluşturma fonksiyonu
MatchStatementNode* match_statement_create(ExpressionNode* expression, MatchCaseNode* first_case, StatementListNode* default_case);

#endif