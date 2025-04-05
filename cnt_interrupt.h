#ifndef CNT_INTERRUPT_H
#define CNT_INTERRUPT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Kesme Önceliği
typedef enum {
    INTERRUPT_PRIORITY_LOW,
    INTERRUPT_PRIORITY_MEDIUM,
    INTERRUPT_PRIORITY_HIGH,
    // ... diğer kesme öncelikleri ...
} Interrupt_PriorityTypeDef;

// Kesme Yapılandırma Yapısı
typedef struct {
    uint32_t InterruptNumber;
    Interrupt_PriorityTypeDef Priority;
    void (*Callback)(void);
} Interrupt_InitTypeDef;

// Kesme Başlatma Fonksiyonu
bool Interrupt_Init(Interrupt_InitTypeDef* init);

// Kesme Etkinleştirme Fonksiyonu
bool Interrupt_Enable(uint32_t interruptNumber);

// Kesme Devre Dışı Bırakma Fonksiyonu
bool Interrupt_Disable(uint32_t interruptNumber);

#endif