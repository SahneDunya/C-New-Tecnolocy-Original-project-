#ifndef CNT_COMPILETIME_H
#define CNT_COMPILETIME_H

#include <stdbool.h>
#include <stddef.h>

// Derleme Zamanı Hata Türleri
typedef enum {
    COMPILE_ERROR_OWNERSHIP,
    COMPILE_ERROR_BORROW,
    COMPILE_ERROR_LIFETIME,
    // ... diğer hata türleri ...
} CompileErrorType;

// Derleme Zamanı Hata Yapısı
typedef struct CompileError {
    CompileErrorType type;
    const char* message;
    // ... diğer hata bilgileri ...
} CompileError;

// Derleme Zamanı Hata Raporlama Fonksiyonu
void compile_error(CompileError error);

// Derleme Zamanı Sahiplik Kontrol Fonksiyonu
bool compile_check_ownership(void* owner);

// Derleme Zamanı Ödünç Alma Kontrol Fonksiyonu
bool compile_check_borrow(void* borrow);

// Derleme Zamanı Yaşam Süresi Kontrol Fonksiyonu
bool compile_check_lifetime(void* lifetime);

#endif