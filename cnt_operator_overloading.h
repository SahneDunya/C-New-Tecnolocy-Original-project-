#ifndef CNT_OPERATOR_OVERLOADING_H
#define CNT_OPERATOR_OVERLOADING_H

#include <stdbool.h>
#include "cnt_operator.h"        // OperatorCode için
#include "cnt_type.h"            // Type yapısı için
#include "cnt_operator_implementation.h" // Operatör implementasyon fonksiyon tipleri için

// Aşırı Yüklenmiş Operatör Implementasyonu Yapısı
typedef struct OverloadedOperatorImpl {
    OperatorCode operator_code;
    Type* left_operand_type;
    Type* right_operand_type; // Unary operatörler için NULL olabilir
    BinaryOperatorFunc binary_func; // İkili operatörler için implementasyon fonksiyonu
    UnaryOperatorFunc unary_func;   // Tekli operatörler için implementasyon fonksiyonu
    struct OverloadedOperatorImpl* next;
} OverloadedOperatorImpl;

// Aşırı Yüklenmiş Operatör Implementasyonlarını Saklayan Liste
typedef struct OverloadedOperatorRegistry {
    OverloadedOperatorImpl* implementations;
} OverloadedOperatorRegistry;

// Aşırı Yüklenmiş Operatör Kayıt Fonksiyonları
OverloadedOperatorRegistry* create_overloaded_operator_registry();
void register_overloaded_binary_operator(OverloadedOperatorRegistry* registry, OperatorCode operator_code, Type* left_type, Type* right_type, BinaryOperatorFunc func);
void register_overloaded_unary_operator(OverloadedOperatorRegistry* registry, OperatorCode operator_code, Type* operand_type, UnaryOperatorFunc func);

// Aşırı Yüklenmiş Operatör Çözümleme Fonksiyonları
BinaryOperatorFunc resolve_overloaded_binary_operator(const OverloadedOperatorRegistry* registry, OperatorCode operator_code, const Type* left_type, const Type* right_type);
UnaryOperatorFunc resolve_overloaded_unary_operator(const OverloadedOperatorRegistry* registry, OperatorCode operator_code, const Type* operand_type);

#endif