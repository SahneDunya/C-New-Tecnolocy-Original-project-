#include "cnt_loop.h"
#include <stdlib.h>

ForLoopNode* for_loop_create(ExpressionNode* initialization, ExpressionNode* condition, ExpressionNode* increment, StatementListNode* body) {
    ForLoopNode* node = (ForLoopNode*)malloc(sizeof(ForLoopNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_FOR_LOOP;
    node->initialization = initialization;
    node->condition = condition;
    node->increment = increment;
    node->body = body;
    return node;
}

WhileLoopNode* while_loop_create(ExpressionNode* condition, StatementListNode* body) {
    WhileLoopNode* node = (WhileLoopNode*)malloc(sizeof(WhileLoopNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_WHILE_LOOP;
    node->condition = condition;
    node->body = body;
    return node;
}

 RangeBasedForLoopNode* range_based_for_loop_create(IdentifierNode* variable, ExpressionNode* iterable, StatementListNode* body) {
     RangeBasedForLoopNode* node = (RangeBasedForLoopNode*)malloc(sizeof(RangeBasedForLoopNode));
     if (!node) {
         perror("Bellek ayırma hatası");
         exit(EXIT_FAILURE);
     }
     node->base.type = AST_TYPE_RANGE_BASED_FOR_LOOP;
     node->variable = variable;
     node->iterable = iterable;
     node->body = body;
     return node;
 }