#include "cnt_unsafe.h"
#include <stdlib.h>

UnsafeBlockNode* unsafe_block_create(StatementListNode* body) {
    UnsafeBlockNode* node = (UnsafeBlockNode*)malloc(sizeof(UnsafeBlockNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_UNSAFE_BLOCK;
    node->body = body;
    return node;
}