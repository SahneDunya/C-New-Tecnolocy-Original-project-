#include "cnt_function_call.h"
#include <stdlib.h>

FunctionCallNode* function_call_create(ExpressionNode* callee, ExpressionNode** arguments, int argument_count) {
    FunctionCallNode* node = (FunctionCallNode*)malloc(sizeof(FunctionCallNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_FUNCTION_CALL;
    node->callee = callee;
    node->arguments = arguments;
    node->argument_count = argument_count;
    return node;
}