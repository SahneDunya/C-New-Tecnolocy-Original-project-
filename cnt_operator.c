#include "cnt_operator.h"

// Operatör Bilgileri Tablosu
static const OperatorInfo operators[] = {
    {OP_PLUS, "+", 6, ASSOCIATIVITY_LEFT, true, OPERATOR_ARITHMETIC},
    {OP_MINUS, "-", 6, ASSOCIATIVITY_LEFT, true, OPERATOR_ARITHMETIC}, // Binary
    {OP_MULTIPLY, "*", 7, ASSOCIATIVITY_LEFT, true, OPERATOR_ARITHMETIC},
    {OP_DIVIDE, "/", 7, ASSOCIATIVITY_LEFT, true, OPERATOR_ARITHMETIC},
    {OP_MODULO, "%", 7, ASSOCIATIVITY_LEFT, true, OPERATOR_ARITHMETIC},
    {OP_EQUAL_EQUAL, "==", 4, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_NOT_EQUAL, "!=", 4, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_GREATER_THAN, ">", 5, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_LESS_THAN, "<", 5, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_GREATER_EQUAL, ">=", 5, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_LESS_EQUAL, "<=", 5, ASSOCIATIVITY_LEFT, true, OPERATOR_COMPARISON},
    {OP_AND, "&&", 3, ASSOCIATIVITY_LEFT, false, OPERATOR_LOGICAL},
    {OP_OR, "||", 2, ASSOCIATIVITY_LEFT, false, OPERATOR_LOGICAL},
    {OP_NOT, "!", 8, ASSOCIATIVITY_RIGHT, false, OPERATOR_LOGICAL}, // Unary
    {OP_ASSIGN, "=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_PLUS_ASSIGN, "+=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_MINUS_ASSIGN, "-=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_MULTIPLY_ASSIGN, "*=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_DIVIDE_ASSIGN, "/=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_MODULO_ASSIGN, "%=", 1, ASSOCIATIVITY_RIGHT, false, OPERATOR_ASSIGNMENT},
    {OP_BITWISE_AND, "&", 6, ASSOCIATIVITY_LEFT, true, OPERATOR_BITWISE},
    {OP_BITWISE_OR, "|", 4, ASSOCIATIVITY_LEFT, true, OPERATOR_BITWISE},
    {OP_BITWISE_XOR, "^", 5, ASSOCIATIVITY_LEFT, true, OPERATOR_BITWISE},
    {OP_BITWISE_NOT, "~", 8, ASSOCIATIVITY_RIGHT, true, OPERATOR_BITWISE}, // Unary
    {OP_LEFT_SHIFT, "<<", 6, ASSOCIATIVITY_LEFT, true, OPERATOR_BITWISE},
    {OP_RIGHT_SHIFT, ">>", 6, ASSOCIATIVITY_LEFT, true, OPERATOR_BITWISE},
    {OP_UNKNOWN, NULL, -1, ASSOCIATIVITY_NONE, false, -1} // Sonlandırıcı
};

const OperatorInfo* get_operator_info(OperatorCode code) {
    for (const OperatorInfo* op = operators; op->code != OP_UNKNOWN; ++op) {
        if (op->code == code) {
            return op;
        }
    }
    return NULL; // Operatör bulunamadı
}