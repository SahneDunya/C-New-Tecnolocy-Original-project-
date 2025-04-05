#ifndef CNT_CIRCULAR_BUFFER_H
#define CNT_CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Dairesel Tampon Yapısı
typedef struct {
    uint8_t* buffer;
    uint32_t bufferSize;
    uint32_t head;
    uint32_t tail;
} CircularBuffer;

// Dairesel Tampon Başlatma Fonksiyonu
bool CircularBuffer_Init(CircularBuffer* buffer, uint8_t* data, uint32_t size);

// Dairesel Tampona Veri Yazma Fonksiyonu
bool CircularBuffer_Write(CircularBuffer* buffer, uint8_t* data, uint32_t size);

// Dairesel Tampondan Veri Okuma Fonksiyonu
uint32_t CircularBuffer_Read(CircularBuffer* buffer, uint8_t* data, uint32_t size);

// Dairesel Tampondaki Veri Sayısını Alma Fonksiyonu
uint32_t CircularBuffer_Available(CircularBuffer* buffer);

// Dairesel Tamponun Boş Olup Olmadığını Kontrol Etme Fonksiyonu
bool CircularBuffer_IsEmpty(CircularBuffer* buffer);

// Dairesel Tamponun Dolu Olup Olmadığını Kontrol Etme Fonksiyonu
bool CircularBuffer_IsFull(CircularBuffer* buffer);

// Dairesel Tamponu Sıfırlama Fonksiyonu
void CircularBuffer_Reset(CircularBuffer* buffer);

#endif