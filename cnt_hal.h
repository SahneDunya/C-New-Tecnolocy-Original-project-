#ifndef CNT_HAL_H
#define CNT_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// HAL Hata Kodları
typedef enum {
    HAL_OK,
    HAL_ERROR,
    HAL_TIMEOUT,
    // ... diğer hata kodları ...
} HAL_StatusTypeDef;

// GPIO Modu
typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_ALTERNATE_FUNCTION,
    GPIO_MODE_ANALOG,
    // ... diğer GPIO modları ...
} GPIO_ModeTypeDef;

// GPIO Çıkış Türü
typedef enum {
    GPIO_OUTPUT_TYPE_PUSH_PULL,
    GPIO_OUTPUT_TYPE_OPEN_DRAIN,
    // ... diğer GPIO çıkış türleri ...
} GPIO_OutputTypeTypeDef;

// GPIO Hız
typedef enum {
    GPIO_SPEED_LOW,
    GPIO_SPEED_MEDIUM,
    GPIO_SPEED_HIGH,
    // ... diğer GPIO hızları ...
} GPIO_SpeedTypeDef;

// GPIO Pull Up/Down
typedef enum {
    GPIO_PULL_NONE,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
    // ... diğer GPIO pull up/down ayarları ...
} GPIO_PullTypeDef;

// GPIO Yapılandırma Yapısı
typedef struct {
    uint32_t Pin;
    GPIO_ModeTypeDef Mode;
    GPIO_OutputTypeTypeDef OutputType;
    GPIO_SpeedTypeDef Speed;
    GPIO_PullTypeDef Pull;
    uint32_t Alternate;
} GPIO_InitTypeDef;

// GPIO Başlatma Fonksiyonu
HAL_StatusTypeDef HAL_GPIO_Init(uint32_t port, GPIO_InitTypeDef* init);

// GPIO Okuma Fonksiyonu
HAL_StatusTypeDef HAL_GPIO_ReadPin(uint32_t port, uint32_t pin, uint8_t* value);

// GPIO Yazma Fonksiyonu
HAL_StatusTypeDef HAL_GPIO_WritePin(uint32_t port, uint32_t pin, uint8_t value);

// ... diğer HAL fonksiyonları ...

#endif