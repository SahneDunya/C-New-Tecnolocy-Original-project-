#ifndef CNT_FUNCTION_PARAMETER_H
#define CNT_FUNCTION_PARAMETER_H

#include <stdbool.h>
#include "cnt_type.h" // Parametre tipi için (Type)

// Fonksiyon Parametresi Yapısı
typedef struct FunctionParameter {
    char* name;         // Parametrenin adı
    Type* type;         // Parametrenin tipi
    bool has_default_value; // Modern özellik: varsayılan değeri var mı?
    // İsteğe bağlı: Varsayılan değerin AST düğümü (ExpressionNode olabilir)
    ExpressionNode* default_value;
    bool is_variadic;    // Modern özellik: değişken sayıda argüman mı alıyor?
    struct FunctionParameter* next; // Bir sonraki parametre için
} FunctionParameter;

// Yeni bir fonksiyon parametresi oluşturma fonksiyonu
FunctionParameter* function_parameter_create(const char* name, Type* type, bool has_default_value, bool is_variadic);

// Yeni bir fonksiyon parametresinin varsayılan değerini ayarlama (isteğe bağlı)
void function_parameter_set_default_value(FunctionParameter* parameter, ExpressionNode* defaultValue);

#endif