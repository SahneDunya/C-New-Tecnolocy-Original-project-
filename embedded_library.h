#ifndef EMBEDDED_LIBRARY_H
#define EMBEDDED_LIBRARY_H

// Gömülü CNT Kütüphanesi Başlık Dosyaları
#include "cnt_gpio.h"
#include "cnt_uart.h"
#include "cnt_spi.h"
#include "cnt_i2c.h"
#include "cnt_adc.h"
#include "cnt_timer.h"
#include "cnt_interrupt.h"
#include "cnt_memory.h"
// ... diğer gömülü kütüphane başlık dosyaları ...

// Gömülü CNT Kütüphanesi Makroları
#define CNT_EMBEDDED_LIBRARY_VERSION "1.0.0"

// Gömülü CNT Kütüphanesi İşlevleri (Örnek)
void cnt_embedded_library_example_function(uint8_t pin, uint8_t value);

#endif