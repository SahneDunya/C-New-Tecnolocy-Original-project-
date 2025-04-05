#ifndef CNT_ERROR_H
#define CNT_ERROR_H

#include <stdint.h>
#include <stdbool.h>

// CNT Hata Kodları
typedef enum {
    CNT_ERROR_NONE = 0,
    CNT_ERROR_FILE_OPEN_FAILED,
    CNT_ERROR_FILE_READ_FAILED,
    CNT_ERROR_FILE_WRITE_FAILED,
    CNT_ERROR_MEMORY_ALLOCATION_FAILED,
    CNT_ERROR_INVALID_FILE_FORMAT,
    CNT_ERROR_KEY_NOT_FOUND,
    CNT_ERROR_INVALID_ARGUMENT,
    // ... diğer hata kodları ...
} CNTErrorCode;

// CNT Hata Mesajları
const char* CNT_GetErrorMessage(CNTErrorCode errorCode);

// CNT Hata Durumunu Kontrol Etme Fonksiyonu
bool CNT_IsError(CNTErrorCode errorCode);

// CNT Hata Kodunu Alma Fonksiyonu
CNTErrorCode CNT_GetErrorCode(void);

// CNT Hata Kodunu Ayarlama Fonksiyonu
void CNT_SetErrorCode(CNTErrorCode errorCode);

#endif