#ifndef CNT_TOKEN_H
#define CNT_TOKEN_H

#include <stdbool.h>
#include <stddef.h>

// Token Türleri
typedef enum {
    TOKEN_EOF,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATOR, // Noktalama işaretleri (örneğin, ;, {, })
    TOKEN_UNKNOWN
} TokenType;

// Token Yapısı
typedef struct {
    TokenType type;
    char* value; // Token'ın metinsel değeri
    size_t line;
    size_t column;
} Token;

#endif