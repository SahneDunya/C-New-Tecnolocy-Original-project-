#ifndef CNT_FUNCTION_SIGNATURE_H
#define CNT_FUNCTION_SIGNATURE_H

#include <stdbool.h>
#include "cnt_type.h"            // Parametre tipleri için (Type)
#include "cnt_function_parameter.h" // Fonksiyon parametre listesi için (FunctionParameter)

// Fonksiyon İmzası Yapısı
typedef struct FunctionSignature {
    char* name;                 // Fonksiyonun adı
    Type** parameter_types;     // Parametre tiplerinin dizisi
    int parameter_count;        // Parametre sayısı
} FunctionSignature;

// Yeni bir fonksiyon imzası oluşturma fonksiyonu (FunctionNode'dan)
FunctionSignature* function_signature_create(const char* name, FunctionParameter* parameters);

// İki fonksiyon imzasının aynı olup olmadığını kontrol eden fonksiyon
bool function_signatures_equal(const FunctionSignature* sig1, const FunctionSignature* sig2);

// Bir fonksiyon imzasını yazdırma fonksiyonu (debug için)
void function_signature_print(const FunctionSignature* signature);

#endif