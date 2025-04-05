#ifndef CNT_FUNCTION_CALL_H
#define CNT_FUNCTION_CALL_H

#include <stdbool.h>
#include "cnt_ast.h"        // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_expression.h"   // Fonksiyona geçirilen argümanlar için (ExpressionNode)

// Fonksiyon Çağrısı AST Düğümü
typedef struct FunctionCallNode {
    ASTNode base;
    ExpressionNode* callee;       // Çağrılan fonksiyonu temsil eden ifade (identifier veya fonksiyon pointer'ı olabilir)
    ExpressionNode** arguments;   // Fonksiyona geçirilen argümanların listesi (dizi)
    int argument_count;        // Argüman sayısı
} FunctionCallNode;

// Yeni bir fonksiyon çağrısı AST düğümü oluşturma fonksiyonu
FunctionCallNode* function_call_create(ExpressionNode* callee, ExpressionNode** arguments, int argument_count);

#endif