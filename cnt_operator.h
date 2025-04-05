#ifndef CNT_OPERATOR_H
#define CNT_OPERATOR_H

#include <stdbool.h>

// Operatör Tipleri (isteğe bağlı, kategorize etmek için)
typedef enum {
    OPERATOR_ARITHMETIC,
    OPERATOR_COMPARISON,
    OPERATOR_LOGICAL,
    OPERATOR_ASSIGNMENT,
    OPERATOR_BITWISE,
    // ... diğer operatör tipleri
} OperatorType;

// Birleşme Yönü
typedef enum {
    ASSOCIATIVITY_LEFT,
    ASSOCIATIVITY_RIGHT,
    ASSOCIATIVITY_NONE // Örneğin, atama operatörleri için
} OperatorAssociativity;

// Desteklenen Operatörler (enum olarak tanımlanabilir)
typedef enum {
    OP_PLUS, +
    OP_MINUS, - (binary ve unary)
    OP_MULTIPLY, *
    OP_DIVIDE, /
    OP_MODULO, %
    OP_EQUAL_EQUAL, ==
    OP_NOT_EQUAL, !=
    OP_GREATER_THAN, >
    OP_LESS_THAN, <
    OP_GREATER_EQUAL, >=
    OP_LESS_EQUAL, <=
    OP_AND, &&
    OP_OR, ||
    OP_NOT, ! (unary)
    OP_ASSIGN, =
    OP_PLUS_ASSIGN, +=
    OP_MINUS_ASSIGN, -=
    OP_MULTIPLY_ASSIGN, *=
    OP_DIVIDE_ASSIGN, /=
    OP_MODULO_ASSIGN, %=
    OP_BITWISE_AND, &
    OP_BITWISE_OR, |
    OP_BITWISE_XOR, ^
    OP_BITWISE_NOT, ~ (unary)
    OP_LEFT_SHIFT, <<
    OP_RIGHT_SHIFT, >>
    // ... diğer operatörler
    OP_UNKNOWN
} OperatorCode;

// Operatör Bilgisi Yapısı
typedef struct OperatorInfo {
    OperatorCode code;
    const char* symbol;
    int precedence;
    OperatorAssociativity associativity;
    bool overloadable; // Modern özellik: operatör aşırı yüklemesi destekleniyor mu?
    OperatorType type; // İsteğe bağlı: operatörün türü
} OperatorInfo;

// Operatör bilgilerini adına göre getiren fonksiyon (cnt_operator.c içinde tanımlanacak)
const OperatorInfo* get_operator_info(OperatorCode code);

#endif