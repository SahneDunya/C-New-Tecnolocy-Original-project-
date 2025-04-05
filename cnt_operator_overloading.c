#include "cnt_operator_overloading.h"
#include <stdlib.h>
#include <stdio.h>

// Aşırı Yüklenmiş Operatör Kayıt Fonksiyonları

OverloadedOperatorRegistry* create_overloaded_operator_registry() {
    OverloadedOperatorRegistry* registry = (OverloadedOperatorRegistry*)malloc(sizeof(OverloadedOperatorRegistry));
    if (!registry) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    registry->implementations = NULL;
    return registry;
}

void register_overloaded_binary_operator(OverloadedOperatorRegistry* registry, OperatorCode operator_code, Type* left_type, Type* right_type, BinaryOperatorFunc func) {
    OverloadedOperatorImpl* new_impl = (OverloadedOperatorImpl*)malloc(sizeof(OverloadedOperatorImpl));
    if (!new_impl) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    new_impl->operator_code = operator_code;
    new_impl->left_operand_type = left_type;
    new_impl->right_operand_type = right_type;
    new_impl->binary_func = func;
    new_impl->unary_func = NULL; // İkili operatör için tekli fonksiyon olmaz
    new_impl->next = registry->implementations;
    registry->implementations = new_impl;
}

void register_overloaded_unary_operator(OverloadedOperatorRegistry* registry, OperatorCode operator_code, Type* operand_type, UnaryOperatorFunc func) {
    OverloadedOperatorImpl* new_impl = (OverloadedOperatorImpl*)malloc(sizeof(OverloadedOperatorImpl));
    if (!new_impl) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    new_impl->operator_code = operator_code;
    new_impl->left_operand_type = operand_type;
    new_impl->right_operand_type = NULL; // Tekli operatör için sağ operand olmaz
    new_impl->binary_func = NULL; // Tekli operatör için ikili fonksiyon olmaz
    new_impl->unary_func = func;
    new_impl->next = registry->implementations;
    registry->implementations = new_impl;
}

// Aşırı Yüklenmiş Operatör Çözümleme Fonksiyonları

BinaryOperatorFunc resolve_overloaded_binary_operator(const OverloadedOperatorRegistry* registry, OperatorCode operator_code, const Type* left_type, const Type* right_type) {
    OverloadedOperatorImpl* current = registry->implementations;
    while (current != NULL) {
        if (current->operator_code == operator_code &&
            types_are_equal(current->left_operand_type, left_type) &&
            types_are_equal(current->right_operand_type, right_type)) {
            return current->binary_func;
        }
        current = current->next;
    }
    return NULL; // Belirtilen tipler için aşırı yüklenmiş operatör bulunamadı
}

UnaryOperatorFunc resolve_overloaded_unary_operator(const OverloadedOperatorRegistry* registry, OperatorCode operator_code, const Type* operand_type) {
    OverloadedOperatorImpl* current = registry->implementations;
    while (current != NULL) {
        if (current->operator_code == operator_code &&
            types_are_equal(current->left_operand_type, operand_type) &&
            current->right_operand_type == NULL) { // Tekli operatörde sağ operand olmaz
            return current->unary_func;
        }
        current = current->next;
    }
    return NULL; // Belirtilen tip için aşırı yüklenmiş operatör bulunamadı
}