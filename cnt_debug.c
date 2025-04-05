#include "cnt_debug.h"
#include <stdarg.h>
#include <stdio.h>

// Geçerli Hata Ayıklama Seviyesi
static DebugLevel currentDebugLevel = DEBUG_LEVEL_INFO;

// Hata Ayıklama Seviyesini Ayarlama Fonksiyonu
void Debug_SetLevel(DebugLevel level) {
    currentDebugLevel = level;
}

// Hata Ayıklama Mesajı Gönderme Fonksiyonu
void Debug_Print(DebugLevel level, const char* message, ...) {
    if (level <= currentDebugLevel) {
        va_list args;
        va_start(args, message);
        vprintf(message, args);
        va_end(args);
    }
}

// Hata Durumunu Yönetme Fonksiyonu
void Debug_Assert(bool condition, const char* message, ...) {
    if (!condition) {
        va_list args;
        va_start(args, message);
        vprintf(message, args);
        va_end(args);
        // Hata durumunda yapılacak işlemleri buraya ekleyin (örneğin, sistemi durdurma)
        while (1); // Sonsuz döngü (örnek)
    }
}