#include "cnt_unsafe_rules.h"
#include "cnt_ast.h"
#include "cnt_expression.h"
#include "cnt_type.h"
#include <stdio.h>

bool is_unsafe_operation(ASTNode* node, UnsafeOperationType* out_type) {
    if (node == NULL) {
        if (out_type) *out_type = UNSAFE_OPERATION_NONE;
        return false;
    }

    switch (node->type) {
        case AST_TYPE_DEREFERENCE_EXPRESSION:
            if (out_type) *out_type = UNSAFE_OPERATION_POINTER_DEREFERENCE;
            return true;

        case AST_TYPE_CAST_EXPRESSION: {
            CastExpressionNode* cast_node = (CastExpressionNode*)node;
            // Burada daha karmaşık bir kontrol yapılabilir, örneğin:
            // - Bir pointer'dan farklı bir pointer tipine dönüştürme güvenli olabilir.
            // - Sayısal tipler arasında dönüşüm genellikle güvenlidir.
            // - Ancak, bir pointer'ı bir tamsayıya veya tam tersini dönüştürmek genellikle güvenli değildir.
            if (cast_node->target_type && cast_node->expression &&
                (cast_node->target_type->kind == TYPE_KIND_POINTER) &&
                (cast_node->expression->base.type == AST_TYPE_IDENTIFIER_EXPRESSION ||
                 cast_node->expression->base.type == AST_TYPE_INTEGER_LITERAL)) { // Örnek: pointer'ı doğrudan bir sayıya cast etme
                if (out_type) *out_type = UNSAFE_OPERATION_UNSAFE_CAST;
                return true;
            }
            break;
        }

        case AST_TYPE_FUNCTION_CALL: {
            // Burada fonksiyonun güvenli olup olmadığını (örneğin, yabancı bir fonksiyon olup olmadığını) kontrol edebilirsiniz.
            // Bu bilgi fonksiyon tanımında tutulabilir.
            // Örnek olarak, fonksiyon adının belirli bir prefix ile başladığını varsayalım (örn. "ffi_").
            FunctionCallNode* call_node = (FunctionCallNode*)node;
            if (call_node->callee && call_node->callee->base.type == AST_TYPE_IDENTIFIER_EXPRESSION) {
                IdentifierExpressionNode* id_node = (IdentifierExpressionNode*)call_node->callee;
                if (strncmp(id_node->name, "ffi_", 4) == 0) {
                    if (out_type) *out_type = UNSAFE_OPERATION_FOREIGN_FUNCTION_CALL;
                    return true;
                }
            }
            break;
        }

        case AST_TYPE_BINARY_EXPRESSION: {
            BinaryExpressionNode* bin_node = (BinaryExpressionNode*)node;
            // Örnek: Bitwise AND veya OR operatörleri non-integer tipler üzerinde kullanılıyorsa
            if ((bin_node->operator == BINARY_OP_BITWISE_AND || bin_node->operator == BINARY_OP_BITWISE_OR) &&
                bin_node->left && bin_node->right &&
                (bin_node->left->evaluated_type == NULL || bin_node->left->evaluated_type->kind != TYPE_KIND_INTEGER ||
                 bin_node->right->evaluated_type == NULL || bin_node->right->evaluated_type->kind != TYPE_KIND_INTEGER)) {
                if (out_type) *out_type = UNSAFE_OPERATION_BITWISE_ON_NON_INTEGER;
                return true;
            }
            break;
        }

        // ... diğer AST düğüm türleri için kontroller eklenebilir ...

        default:
            if (out_type) *out_type = UNSAFE_OPERATION_NONE;
            return false;
    }

    if (out_type) *out_type = UNSAFE_OPERATION_NONE;
    return false;
}