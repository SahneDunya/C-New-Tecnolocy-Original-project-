#include "cnt_match_statement.h"
#include <stdlib.h>

MatchCaseNode* match_case_create(ExpressionNode* pattern, StatementListNode* body) {
    MatchCaseNode* node = (MatchCaseNode*)malloc(sizeof(MatchCaseNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_MATCH_CASE;
    node->pattern = pattern;
    node->body = body;
    node->next = NULL;
    return node;
}

MatchStatementNode* match_statement_create(ExpressionNode* expression, MatchCaseNode* first_case, StatementListNode* default_case) {
    MatchStatementNode* node = (MatchStatementNode*)malloc(sizeof(MatchStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_MATCH_STATEMENT;
    node->expression = expression;
    node->first_case = first_case;
    node->default_case = default_case;
    return node;
}