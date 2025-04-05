#ifndef CNT_ADC_H
#define CNT_ADC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ADC Çözünürlüğü
typedef enum {
    ADC_RESOLUTION_8BIT,
    ADC_RESOLUTION_10BIT,
    ADC_RESOLUTION_12BIT,
    // ... diğer ADC çözünürlükleri ...
} ADC_ResolutionTypeDef;

// ADC Hizalama
typedef enum {
    ADC_DATA_ALIGN_RIGHT,
    ADC_DATA_ALIGN_LEFT,
    // ... diğer ADC hizalamaları ...
} ADC_DataAlignTypeDef;

// ADC Yapılandırma Yapısı
typedef struct {
    ADC_ResolutionTypeDef Resolution;
    ADC_DataAlignTypeDef DataAlign;
    uint32_t SamplingTime;
} ADC_InitTypeDef;

// ADC Başlatma Fonksiyonu
bool ADC_Init(uint32_t port, ADC_InitTypeDef* init);

// ADC Dönüşümü Başlatma Fonksiyonu
bool ADC_StartConversion(uint32_t port, uint32_t channel);

// ADC Dönüşüm Sonucunu Alma Fonksiyonu
bool ADC_GetValue(uint32_t port, uint32_t channel, uint32_t* value);

#endif