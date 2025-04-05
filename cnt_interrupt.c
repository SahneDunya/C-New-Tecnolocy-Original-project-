#include "cnt_interrupt.h"

// Kesme Başlatma Fonksiyonu
bool Interrupt_Init(Interrupt_InitTypeDef* init) {
    // Donanım bağımlı kesme başlatma mantığı
    // ... kesme başlatma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// Kesme Etkinleştirme Fonksiyonu
bool Interrupt_Enable(uint32_t interruptNumber) {
    // Donanım bağımlı kesme etkinleştirme mantığı
    // ... kesme etkinleştirme mantığı ...

    // Başarılı ise true döndür
    return true;
}

// Kesme Devre Dışı Bırakma Fonksiyonu
bool Interrupt_Disable(uint32_t interruptNumber) {
    // Donanım bağımlı kesme devre dışı bırakma mantığı
    // ... kesme devre dışı bırakma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// Kesme İşleyicisi (Örnek)
void Interrupt_IRQHandler(uint32_t interruptNumber) {
    // Kesme işleme mantığı
    // ... kesme işleme mantığı ...
}