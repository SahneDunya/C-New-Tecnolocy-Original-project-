#ifndef CNT_ERROR_H
#define CNT_ERROR_H

#include <stddef.h>

// Hata Türleri (isteğe bağlı)
typedef enum {
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_INTERNAL
    // ... diğer hata türleri ...
} ErrorType;

// Hata Bilgisi Yapısı
typedef struct ErrorInfo {
    ErrorType type;
    size_t line;
    size_t column;
    char* message;
} ErrorInfo;

// Hata Raporlama Fonksiyonu (genel amaçlı)
void report_error(ErrorType type, size_t line, size_t column, const char* message);

#endif