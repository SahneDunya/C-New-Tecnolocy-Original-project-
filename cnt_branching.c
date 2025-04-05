#include "cnt_branching.h"
#include <stdlib.h>

BreakStatementNode* break_statement_create() {
    BreakStatementNode* node = (BreakStatementNode*)malloc(sizeof(BreakStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_BREAK_STATEMENT;
    return node;
}

ContinueStatementNode* continue_statement_create() {
    ContinueStatementNode* node = (ContinueStatementNode*)malloc(sizeof(ContinueStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_CONTINUE_STATEMENT;
    return node;
}

ReturnStatementNode* return_statement_create(ExpressionNode* value) {
    ReturnStatementNode* node = (ReturnStatementNode*)malloc(sizeof(ReturnStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_RETURN_STATEMENT;
    node->value = value;
    return node;
}