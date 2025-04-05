#include "cnt_lexer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Anahtar Kelimeler (modern özellikler eklenebilir)
static const char* keywords[] = {
    "int", "char", "float", "bool", "void",
    "if", "else", "while", "for", "return",
    "struct", "enum", "typedef",
    "module", "import", // Modern özellikler için örnekler
    "interface", "impl",
    "own", "borrow",
    NULL
};

Lexer* lexer_init(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    lexer->source = source;
    lexer->current_pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

static bool is_keyword(const char* str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(keywords[i], str) == 0) {
            return true;
        }
    }
    return false;
}

static Token* create_token(TokenType type, const char* value, size_t line, size_t column) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (token == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    token->type = type;
    token->value = strdup(value);
    token->line = line;
    token->column = column;
    return token;
}

void token_free(Token* token) {
    free(token->value);
    free(token);
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->current_pos];
}

static char consume(Lexer* lexer) {
    if (lexer->source[lexer->current_pos] == '\0') {
        return '\0';
    }
    char c = lexer->source[lexer->current_pos++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

Token* lexer_get_next_token(Lexer* lexer) {
    while (isspace(peek(lexer))) {
        consume(lexer);
    }

    if (peek(lexer) == '\0') {
        return create_token(TOKEN_EOF, "", lexer->line, lexer->column);
    }

    size_t start_line = lexer->line;
    size_t start_column = lexer->column;

    if (isalpha(peek(lexer)) || peek(lexer) == '_') {
        size_t start = lexer->current_pos;
        while (isalnum(peek(lexer)) || peek(lexer) == '_') {
            consume(lexer);
        }
        size_t length = lexer->current_pos - start;
        char* identifier = (char*)malloc(length + 1);
        strncpy(identifier, lexer->source + start, length);
        identifier[length] = '\0';

        if (is_keyword(identifier)) {
            Token* token = create_token(TOKEN_KEYWORD, identifier, start_line, start_column);
            free(identifier);
            return token;
        } else {
            Token* token = create_token(TOKEN_IDENTIFIER, identifier, start_line, start_column);
            free(identifier);
            return token;
        }
    }

    if (isdigit(peek(lexer))) {
        size_t start = lexer->current_pos;
        bool is_float = false;
        while (isdigit(peek(lexer))) {
            consume(lexer);
        }
        if (peek(lexer) == '.') {
            is_float = true;
            consume(lexer);
            while (isdigit(peek(lexer))) {
                consume(lexer);
            }
        }
        size_t length = lexer->current_pos - start;
        char* literal = (char*)malloc(length + 1);
        strncpy(literal, lexer->source + start, length);
        literal[length] = '\0';
        return create_token(is_float ? TOKEN_FLOAT_LITERAL : TOKEN_INTEGER_LITERAL, literal, start_line, start_column);
    }

    if (peek(lexer) == '"') {
        consume(lexer); // Açılış tırnağını tüket
        size_t start = lexer->current_pos;
        while (peek(lexer) != '"' && peek(lexer) != '\0') {
            consume(lexer);
        }
        size_t length = lexer->current_pos - start;
        char* literal = (char*)malloc(length + 1);
        strncpy(literal, lexer->source + start, length);
        literal[length] = '\0';
        consume(lexer); // Kapanış tırnağını tüket
        return create_token(TOKEN_STRING_LITERAL, literal, start_line, start_column);
    }

    // Operatörler ve Noktalama İşaretleri (basit bir yaklaşım)
    char current = consume(lexer);
    char str[2] = {current, '\0'};
    TokenType type = TOKEN_UNKNOWN;

    switch (current) {
        case '+': case '-': case '*': case '/': case '%':
        case '=': case '!': case '<': case '>':
        case '&': case '|': case '^':
            type = TOKEN_OPERATOR;
            break;
        case ';': case ',': case '(': case ')': case '{': case '}': case '[': case ']':
            type = TOKEN_PUNCTUATOR;
            break;
        default:
            fprintf(stderr, "Bilinmeyen token: '%c' satır: %zu, sütun: %zu\n", current, start_line, start_column);
            type = TOKEN_UNKNOWN;
            break;
    }

    return create_token(type, str, start_line, start_column);
}