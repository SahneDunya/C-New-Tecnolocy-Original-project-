#ifndef CNT_DEBUG_H
#define CNT_DEBUG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Hata Ayıklama Seviyeleri
typedef enum {
    DEBUG_LEVEL_NONE,
    DEBUG_LEVEL_ERROR,
    DEBUG_LEVEL_WARNING,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DEBUG,
    DEBUG_LEVEL_VERBOSE
} DebugLevel;

// Hata Ayıklama Seviyesini Ayarlama Fonksiyonu
void Debug_SetLevel(DebugLevel level);

// Hata Ayıklama Mesajı Gönderme Fonksiyonu
void Debug_Print(DebugLevel level, const char* message, ...);

// Hata Durumunu Yönetme Fonksiyonu
void Debug_Assert(bool condition, const char* message, ...);

#endif