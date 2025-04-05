#include "cnt_pattern.h"
#include <stdlib.h>
#include <string.h>

// Yeni desen oluşturma fonksiyonlarının implementasyonları

LiteralPattern* literal_pattern_create(LiteralExpressionNode* value) {
    LiteralPattern* pattern = (LiteralPattern*)malloc(sizeof(LiteralPattern));
    if (!pattern) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    pattern->base.type = PATTERN_LITERAL;
    pattern->value = value;
    return pattern;
}

VariablePattern* variable_pattern_create(const char* name) {
    VariablePattern* pattern = (VariablePattern*)malloc(sizeof(VariablePattern));
    if (!pattern) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    pattern->base.type = PATTERN_VARIABLE;
    pattern->name = strdup(name);
    return pattern;
}

WildcardPattern* wildcard_pattern_create() {
    WildcardPattern* pattern = (WildcardPattern*)malloc(sizeof(WildcardPattern));
    if (!pattern) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    pattern->base.type = PATTERN_WILDCARD;
    return pattern;
}

TypeCheckPattern* type_check_pattern_create(Type* expected_type) {
    TypeCheckPattern* pattern = (TypeCheckPattern*)malloc(sizeof(TypeCheckPattern));
    if (!pattern) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    pattern->base.type = PATTERN_TYPE_CHECK;
    pattern->expected_type = expected_type;
    return pattern;
}

// Bir değerin bir desenle eşleşip eşleşmediğini kontrol eden fonksiyonun implementasyonu
bool pattern_match(const Pattern* pattern, const ExpressionNode* value) {
    if (!pattern || !value) {
        return false;
    }

    switch (pattern->type) {
        case PATTERN_LITERAL: {
            const LiteralPattern* literal_pattern = (const LiteralPattern*)pattern;
            // Burada değerin ve desenin literal değerlerini karşılaştırmanız gerekecek.
            // Bu, değerlerin türüne ve içeriğine bağlı olarak değişebilir.
            // Örnek olarak, sadece tam sayı literal'lerini karşılaştıralım.
            if (value->type == AST_TYPE_LITERAL_EXPRESSION) {
                const LiteralExpressionNode* literal_value = (const LiteralExpressionNode*)value;
                if (literal_pattern->value->literal_type == LITERAL_INTEGER &&
                    literal_value->literal_type == LITERAL_INTEGER &&
                    literal_pattern->value->int_value == literal_value->int_value) {
                    return true;
                } else if (literal_pattern->value->literal_type == LITERAL_BOOLEAN &&
                           literal_value->literal_type == LITERAL_BOOLEAN &&
                           literal_pattern->value->bool_value == literal_value->bool_value) {
                    return true;
                } else if (literal_pattern->value->literal_type == LITERAL_STRING &&
                           literal_value->literal_type == LITERAL_STRING &&
                           strcmp(literal_pattern->value->string_value, literal_value->string_value) == 0) {
                    return true;
                }
            }
            return false;
        }
        case PATTERN_VARIABLE:
            // Değişken desen her zaman eşleşir. Değer, semantik analiz veya kod üretimi sırasında değişkene bağlanacaktır.
            return true;
        case PATTERN_WILDCARD:
            // Joker karakter deseni her zaman eşleşir.
            return true;
        case PATTERN_TYPE_CHECK: {
            const TypeCheckPattern* type_pattern = (const TypeCheckPattern*)pattern;
            // Burada değerin türünü desenin beklenen türüyle karşılaştırmanız gerekecek.
            // Bu, dilin tip sistemine bağlı olarak değişebilir.
            if (value->evaluated_type && type_pattern->expected_type &&
                types_are_equal(value->evaluated_type, type_pattern->expected_type)) {
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}