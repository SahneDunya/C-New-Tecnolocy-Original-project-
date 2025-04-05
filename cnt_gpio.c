#include "cnt_gpio.h"

// GPIO Başlatma Fonksiyonu
bool GPIO_Init(uint32_t port, GPIO_InitTypeDef* init) {
    // Donanım bağımlı GPIO başlatma mantığı
    // ... GPIO başlatma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// GPIO Okuma Fonksiyonu
bool GPIO_ReadPin(uint32_t port, uint32_t pin, uint8_t* value) {
    // Donanım bağımlı GPIO okuma mantığı
    // ... GPIO okuma mantığı ...

    // Okunan değeri value'ya yaz
    *value = 0; // örnek değer

    // Başarılı ise true döndür
    return true;
}

// GPIO Yazma Fonksiyonu
bool GPIO_WritePin(uint32_t port, uint32_t pin, uint8_t value) {
    // Donanım bağımlı GPIO yazma mantığı
    // ... GPIO yazma mantığı ...

    // Başarılı ise true döndür
    return true;
}