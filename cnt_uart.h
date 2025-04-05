#ifndef CNT_UART_H
#define CNT_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// UART Veri Bitiş Sayısı
typedef enum {
    UART_DATA_BITS_8,
    UART_DATA_BITS_9,
    // ... diğer veri bitiş sayıları ...
} UART_DataBitsTypeDef;

// UART Parite
typedef enum {
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
    // ... diğer parite ayarları ...
} UART_ParityTypeDef;

// UART Durdurma Bitiş Sayısı
typedef enum {
    UART_STOP_BITS_1,
    UART_STOP_BITS_2,
    // ... diğer durdurma bitiş sayıları ...
} UART_StopBitsTypeDef;

// UART Yapılandırma Yapısı
typedef struct {
    uint32_t BaudRate;
    UART_DataBitsTypeDef DataBits;
    UART_ParityTypeDef Parity;
    UART_StopBitsTypeDef StopBits;
} UART_InitTypeDef;

// UART Başlatma Fonksiyonu
bool UART_Init(uint32_t port, UART_InitTypeDef* init);

// UART Veri Gönderme Fonksiyonu
bool UART_Transmit(uint32_t port, uint8_t* data, uint32_t size);

// UART Veri Alma Fonksiyonu
bool UART_Receive(uint32_t port, uint8_t* data, uint32_t size);

#endif