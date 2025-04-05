#ifndef CNT_UNSAFE_RULES_H
#define CNT_UNSAFE_RULES_H

#include <stdbool.h>

// Güvenli Olmayan Operasyon Türleri
typedef enum {
    UNSAFE_OPERATION_NONE,
    UNSAFE_OPERATION_POINTER_DEREFERENCE,
    UNSAFE_OPERATION_UNSAFE_CAST,
    UNSAFE_OPERATION_FOREIGN_FUNCTION_CALL,
    UNSAFE_OPERATION_BITWISE_ON_NON_INTEGER,
    // ... diğer güvenli olmayan operasyon türleri eklenebilir ...
} UnsafeOperationType;

// Bir AST düğümünün güvenli olmayan bir operasyon içerip içermediğini kontrol eden fonksiyonun prototipi
bool is_unsafe_operation(ASTNode* node, UnsafeOperationType* out_type);

#endif