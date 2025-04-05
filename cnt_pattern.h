#ifndef CNT_PATTERN_H
#define CNT_PATTERN_H

#include <stdbool.h>
#include "cnt_type.h"         // Tip kontrolü için (Type)
#include "cnt_expression.h"   // Sabit değer desenleri için (LiteralExpressionNode)

// Desen Türleri
typedef enum {
    PATTERN_LITERAL, "true"
    PATTERN_VARIABLE, "x"
    PATTERN_WILDCARD, "_"
    PATTERN_TYPE_CHECK, "string"
    // İleride eklenebilecek diğer desen türleri (örn: tuple desenleri, list desenleri)
} PatternType;

// Temel Desen Yapısı
typedef struct Pattern {
    PatternType type;
} Pattern;

// Sabit Değer Deseni
typedef struct LiteralPattern {
    Pattern base;
    LiteralExpressionNode* value; // Eşleştirilecek sabit değer
} LiteralPattern;

// Değişken Deseni
typedef struct VariablePattern {
    Pattern base;
    char* name; // Değişkenin adı
} VariablePattern;

// Joker Karakter Deseni
typedef struct WildcardPattern {
    Pattern base;
} WildcardPattern;

// Tip Kontrol Deseni
typedef struct TypeCheckPattern {
    Pattern base;
    Type* expected_type; // Eşleşmesi beklenen tip
} TypeCheckPattern;

// Yeni desen oluşturma fonksiyonlarının prototipleri
LiteralPattern* literal_pattern_create(LiteralExpressionNode* value);
VariablePattern* variable_pattern_create(const char* name);
WildcardPattern* wildcard_pattern_create();
TypeCheckPattern* type_check_pattern_create(Type* expected_type);

// Bir değerin bir desenle eşleşip eşleşmediğini kontrol eden fonksiyonun prototipi
bool pattern_match(const Pattern* pattern, const ExpressionNode* value);

#endif