#include "cnt_error.h"

// Geçerli CNT Hata Kodu
static CNTErrorCode currentErrorCode = CNT_ERROR_NONE;

// CNT Hata Mesajları
const char* CNT_GetErrorMessage(CNTErrorCode errorCode) {
    switch (errorCode) {
        case CNT_ERROR_NONE:
            return "Hata yok";
        case CNT_ERROR_FILE_OPEN_FAILED:
            return "Dosya açma hatası";
        case CNT_ERROR_FILE_READ_FAILED:
            return "Dosya okuma hatası";
        case CNT_ERROR_FILE_WRITE_FAILED:
            return "Dosya yazma hatası";
        case CNT_ERROR_MEMORY_ALLOCATION_FAILED:
            return "Bellek ayırma hatası";
        case CNT_ERROR_INVALID_FILE_FORMAT:
            return "Geçersiz dosya formatı";
        case CNT_ERROR_KEY_NOT_FOUND:
            return "Anahtar bulunamadı";
        case CNT_ERROR_INVALID_ARGUMENT:
            return "Geçersiz argüman";
        // ... diğer hata mesajları ...
        default:
            return "Bilinmeyen hata";
    }
}

// CNT Hata Durumunu Kontrol Etme Fonksiyonu
bool CNT_IsError(CNTErrorCode errorCode) {
    return errorCode != CNT_ERROR_NONE;
}

// CNT Hata Kodunu Alma Fonksiyonu
CNTErrorCode CNT_GetErrorCode(void) {
    return currentErrorCode;
}

// CNT Hata Kodunu Ayarlama Fonksiyonu
void CNT_SetErrorCode(CNTErrorCode errorCode) {
    currentErrorCode = errorCode;
}