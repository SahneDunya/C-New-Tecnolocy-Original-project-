#ifndef CNT_OPERATOR_PRECEDENCE_H
#define CNT_OPERATOR_PRECEDENCE_H

// Operatör Öncelik Seviyeleri (C standardına yakın bir sıralama)

// En düşük öncelik (virgül operatörü - CNT'de olmayabilir)
#define PREC_COMMA          1

// Atama operatörleri (=, +=, -=, vb.)
#define PREC_ASSIGNMENT     2

// Koşullu operatör (?: - CNT'de olmayabilir)
#define PREC_CONDITIONAL    3

// Mantıksal OR operatörü (||)
#define PREC_LOGICAL_OR     4

// Mantıksal AND operatörü (&&)
#define PREC_LOGICAL_AND    5

// Bitwise OR operatörü (|)
#define PREC_BITWISE_OR     6

// Bitwise XOR operatörü (^)
#define PREC_BITWISE_XOR    7

// Bitwise AND operatörü (&)
#define PREC_BITWISE_AND    8

// Eşitlik ve eşitsizlik operatörleri (==, !=)
#define PREC_EQUALITY         9

// İlişkisel operatörler (<, >, <=, >=)
#define PREC_RELATIONAL       10

// Bit kaydırma operatörleri (<<, >>)
#define PREC_SHIFT            11

// Toplama ve çıkarma operatörleri (+, -)
#define PREC_ADDITION         12

// Çarpma, bölme ve modulo operatörleri (*, /, %)
#define PREC_MULTIPLICATION   13

// Tekli operatörler (!, +, -, ~) ve type casting
#define PREC_UNARY            14

// Postfix artırma/azaltma (++, --) ve diğer postfix operatörleri ([], ., ->)
#define PREC_POSTFIX          15

// En yüksek öncelik (parantezler)
#define PREC_PRIMARY          16

#endif