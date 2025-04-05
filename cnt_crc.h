#ifndef CNT_CRC_H
#define CNT_CRC_H

#include <stdint.h>
#include <stddef.h>

// CRC Hesaplama Fonksiyonu
uint32_t CRC32_Calculate(const uint8_t* data, uint32_t length, uint32_t polynomial, uint32_t initialValue, bool reflectInput, bool reflectOutput, uint32_t finalXor);

#endif