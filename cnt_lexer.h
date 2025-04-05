#ifndef CNT_LEXER_H
#define CNT_LEXER_H

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

// Lexer Yapısı (isteğe bağlı, lexer'ın durumunu tutabilir)
typedef struct {
    const char* source;
    size_t current_pos;
    size_t line;
    size_t column;
} Lexer;

// Lexer'ı Başlatma Fonksiyonu
Lexer* lexer_init(const char* source);

// Bir Sonraki Token'ı Alma Fonksiyonu
Token* lexer_get_next_token(Lexer* lexer);

// Token'ı Serbest Bırakma Fonksiyonu
void token_free(Token* token);

#endif