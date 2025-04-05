#ifndef CNT_MEMORY_H
#define CNT_MEMORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Statik Bellek Havuzu Yapısı
typedef struct {
    uint8_t* buffer;
    uint32_t blockSize;
    uint32_t blockCount;
    uint8_t* freeList;
} MemoryPool;

// Statik Bellek Havuzu Başlatma Fonksiyonu
bool MemoryPool_Init(MemoryPool* pool, uint8_t* buffer, uint32_t blockSize, uint32_t blockCount);

// Statik Bellek Havuzundan Blok Alma Fonksiyonu
void* MemoryPool_Allocate(MemoryPool* pool);

// Statik Bellek Havuzuna Blok İade Etme Fonksiyonu
bool MemoryPool_Free(MemoryPool* pool, void* block);

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

#endif