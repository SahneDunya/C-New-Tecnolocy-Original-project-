#include "cnt_try_catch.h"
#include <stdlib.h>

CatchClauseNode* catch_clause_create(Type* exception_type, StatementListNode* body) {
    CatchClauseNode* node = (CatchClauseNode*)malloc(sizeof(CatchClauseNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_CATCH_CLAUSE;
    node->exception_type = exception_type;
    node->body = body;
    node->next = NULL;
    return node;
}

TryStatementNode* try_statement_create(StatementListNode* try_block, CatchClauseNode* first_catch, StatementListNode* finally_block) {
    TryStatementNode* node = (TryStatementNode*)malloc(sizeof(TryStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_TRY_STATEMENT;
    node->try_block = try_block;
    node->first_catch = first_catch;
    node->finally_block = finally_block;
    return node;
}