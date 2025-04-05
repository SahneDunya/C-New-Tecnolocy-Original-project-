#include "cnt_match.h"
#include <stdlib.h>

MatchCaseNode* match_case_create(Pattern* pattern, StatementListNode* body, MatchCaseNode* next) {
    MatchCaseNode* node = (MatchCaseNode*)malloc(sizeof(MatchCaseNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_MATCH_CASE;
    node->pattern = pattern;
    node->body = body;
    node->next = next;
    return node;
}

MatchStatementNode* match_statement_create(ExpressionNode* expression, MatchCaseNode* first_case, StatementListNode* default_body) {
    MatchStatementNode* node = (MatchStatementNode*)malloc(sizeof(MatchStatementNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_MATCH_STATEMENT;
    node->expression = expression;
    node->first_case = first_case;
    node->default_body = default_body;
    return node;
}