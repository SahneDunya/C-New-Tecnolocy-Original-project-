#ifndef CNT_OPERATOR_IMPLEMENTATION_H
#define CNT_OPERATOR_IMPLEMENTATION_H

#include <stdbool.h>
#include "cnt_type.h" // Type yapısı için

// Operatör Implementasyon Fonksiyon Tipleri (isteğe bağlı, fonksiyon pointerları için)
typedef void* (*BinaryOperatorFunc)(const void* left_operand, const Type* left_type, const void* right_operand, const Type* right_type, Type** result_type);
typedef void* (*UnaryOperatorFunc)(const void* operand, const Type* operand_type, Type** result_type);

// Aritmetik Operatör Implementasyonları
void* add_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* subtract_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* multiply_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* divide_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* modulo_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);

void* add_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* subtract_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* multiply_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* divide_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);

// Karşılaştırma Operatör Implementasyonları (bool döndürür)
void* equals_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* not_equals_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* greater_than_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* less_than_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* greater_equal_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* less_equal_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);

void* equals_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* not_equals_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* greater_than_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* less_than_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* greater_equal_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* less_equal_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);

// Mantıksal Operatör Implementasyonları (bool döndürür)
void* and_bool(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* or_bool(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type);
void* not_bool(const void* operand, const Type* operand_type, Type** result_type);

// ... diğer operatörler ve tipler için implementasyon prototipleri ...

#endif