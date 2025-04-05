#include "cnt_if_statement.h"
#include <stdlib.h>

IfStatementNode* if_statement_create(ExpressionNode* condition, StatementListNode* body) {
    IfStatementNode* node = (IfStatementNode*)malloc(sizeof(IfStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_IF_STATEMENT;
    node->condition = condition;
    node->body = body;
    node->next_else_if = NULL;
    node->else_branch = NULL;
    return node;
}

IfStatementNode* else_if_statement_create(ExpressionNode* condition, StatementListNode* body) {
    IfStatementNode* node = (IfStatementNode*)malloc(sizeof(IfStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_ELSE_IF_STATEMENT;
    node->condition = condition;
    node->body = body;
    node->next_else_if = NULL;
    node->else_branch = NULL;
    return node;
}

ElseStatementNode* else_statement_create(StatementListNode* body) {
    ElseStatementNode* node = (ElseStatementNode*)malloc(sizeof(ElseStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_ELSE_STATEMENT;
    node->body = body;
    return node;
}