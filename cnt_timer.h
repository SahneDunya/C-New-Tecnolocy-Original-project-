#ifndef CNT_TIMER_H
#define CNT_TIMER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Zamanlayıcı Modu
typedef enum {
    TIMER_MODE_ONE_SHOT,
    TIMER_MODE_PERIODIC,
    // ... diğer zamanlayıcı modları ...
} Timer_ModeTypeDef;

// Zamanlayıcı Yapılandırma Yapısı
typedef struct {
    uint32_t Period;
    Timer_ModeTypeDef Mode;
    void (*Callback)(void);
} Timer_InitTypeDef;

// Zamanlayıcı Başlatma Fonksiyonu
bool Timer_Init(uint32_t timer, Timer_InitTypeDef* init);

// Zamanlayıcı Başlatma Fonksiyonu
bool Timer_Start(uint32_t timer);

// Zamanlayıcı Durdurma Fonksiyonu
bool Timer_Stop(uint32_t timer);

#endif