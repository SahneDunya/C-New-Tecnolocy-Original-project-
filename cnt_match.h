#ifndef CNT_MATCH_H
#define CNT_MATCH_H

#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"   // Eşleştirilecek ifade için (ExpressionNode)
#include "cnt_statement.h"    // Case blokları içindeki ifadeler için (StatementListNode)
#include "cnt_pattern.h"      // Desen yapısı için (Pattern)

// Match Case AST Düğümü
typedef struct MatchCaseNode {
    ASTNode base;
    Pattern* pattern;          // Eşleştirilecek desen
    StatementListNode* body; // Bu desenle eşleşirse çalıştırılacak ifadeler
    struct MatchCaseNode* next; // Bir sonraki case için
} MatchCaseNode;

// Match İfadesi AST Düğümü
typedef struct MatchStatementNode {
    ASTNode base;
    ExpressionNode* expression; // Üzerinde eşleştirme yapılacak ifade
    MatchCaseNode* first_case; // İlk case bloğu
    StatementListNode* default_body; // İsteğe bağlı: hiçbir case eşleşmezse çalıştırılacak ifadeler
} MatchStatementNode;

// Yeni bir Match Case AST düğümü oluşturma fonksiyonu
MatchCaseNode* match_case_create(Pattern* pattern, StatementListNode* body, MatchCaseNode* next);

// Yeni bir Match ifadesi AST düğümü oluşturma fonksiyonu
MatchStatementNode* match_statement_create(ExpressionNode* expression, MatchCaseNode* first_case, StatementListNode* default_body);

#endif