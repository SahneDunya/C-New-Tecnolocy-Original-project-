#ifndef CNT_SPI_H
#define CNT_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// SPI Modu
typedef enum {
    SPI_MODE_MASTER,
    SPI_MODE_SLAVE,
    // ... diğer SPI modları ...
} SPI_ModeTypeDef;

// SPI Veri Sırası
typedef enum {
    SPI_DATA_ORDER_MSB_FIRST,
    SPI_DATA_ORDER_LSB_FIRST,
    // ... diğer veri sıraları ...
} SPI_DataOrderTypeDef;

// SPI Saat Fazı
typedef enum {
    SPI_CLOCK_PHASE_1EDGE,
    SPI_CLOCK_PHASE_2EDGE,
    // ... diğer saat fazları ...
} SPI_ClockPhaseTypeDef;

// SPI Saat Kutbu
typedef enum {
    SPI_CLOCK_POLARITY_LOW,
    SPI_CLOCK_POLARITY_HIGH,
    // ... diğer saat kutupları ...
} SPI_ClockPolarityTypeDef;

// SPI Yapılandırma Yapısı
typedef struct {
    SPI_ModeTypeDef Mode;
    SPI_DataOrderTypeDef DataOrder;
    SPI_ClockPhaseTypeDef ClockPhase;
    SPI_ClockPolarityTypeDef ClockPolarity;
    uint32_t BaudRate;
} SPI_InitTypeDef;

// SPI Başlatma Fonksiyonu
bool SPI_Init(uint32_t port, SPI_InitTypeDef* init);

// SPI Veri Gönderme Fonksiyonu
bool SPI_Transmit(uint32_t port, uint8_t* data, uint32_t size);

// SPI Veri Alma Fonksiyonu
bool SPI_Receive(uint32_t port, uint8_t* data, uint32_t size);

#endif