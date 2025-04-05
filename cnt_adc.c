#include "cnt_adc.h"

// ADC Başlatma Fonksiyonu
bool ADC_Init(uint32_t port, ADC_InitTypeDef* init) {
    // Donanım bağımlı ADC başlatma mantığı
    // ... ADC başlatma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// ADC Dönüşümü Başlatma Fonksiyonu
bool ADC_StartConversion(uint32_t port, uint32_t channel) {
    // Donanım bağımlı ADC dönüşüm başlatma mantığı
    // ... ADC dönüşüm başlatma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// ADC Dönüşüm Sonucunu Alma Fonksiyonu
bool ADC_GetValue(uint32_t port, uint32_t channel, uint32_t* value) {
    // Donanım bağımlı ADC dönüşüm sonucu alma mantığı
    // ... ADC dönüşüm sonucu alma mantığı ...

    // Dönüşüm sonucunu value'ya yaz
    *value = 0; // örnek değer

    // Başarılı ise true döndür
    return true;
}