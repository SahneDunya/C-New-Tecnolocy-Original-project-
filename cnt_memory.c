#include "cnt_memory.h"

// Statik Bellek Havuzu Başlatma Fonksiyonu
bool MemoryPool_Init(MemoryPool* pool, uint8_t* buffer, uint32_t blockSize, uint32_t blockCount) {
    if (pool == NULL || buffer == NULL || blockSize == 0 || blockCount == 0) {
        return false;
    }

    pool->buffer = buffer;
    pool->blockSize = blockSize;
    pool->blockCount = blockCount;
    pool->freeList = buffer;

    // Serbest blok listesini oluştur
    for (uint32_t i = 0; i < blockCount - 1; i++) {
        uint8_t* currentBlock = buffer + i * blockSize;
        uint8_t* nextBlock = buffer + (i + 1) * blockSize;
        *(uint8_t**)currentBlock = nextBlock;
    }
    *(uint8_t**)(buffer + (blockCount - 1) * blockSize) = NULL;

    return true;
}

// Statik Bellek Havuzundan Blok Alma Fonksiyonu
void* MemoryPool_Allocate(MemoryPool* pool) {
    if (pool == NULL || pool->freeList == NULL) {
        return NULL;
    }

    uint8_t* block = pool->freeList;
    pool->freeList = *(uint8_t**)block;

    return block;
}

// Statik Bellek Havuzuna Blok İade Etme Fonksiyonu
bool MemoryPool_Free(MemoryPool* pool, void* block) {
    if (pool == NULL || block == NULL) {
        return false;
    }

    *(uint8_t**)block = pool->freeList;
    pool->freeList = block;

    return true;
}

// Dairesel Tampon Başlatma Fonksiyonu
bool CircularBuffer_Init(CircularBuffer* buffer, uint8_t* data, uint32_t size) {
    if (buffer == NULL || data == NULL || size == 0) {
        return false;
    }

    buffer->buffer = data;
    buffer->bufferSize = size;
    buffer->head = 0;
    buffer->tail = 0;

    return true;
}

// Dairesel Tampona Veri Yazma Fonksiyonu
bool CircularBuffer_Write(CircularBuffer* buffer, uint8_t* data, uint32_t size) {
    if (buffer == NULL || data == NULL || size == 0) {
        return false;
    }

    for (uint32_t i = 0; i < size; i++) {
        uint32_t nextHead = (buffer->head + 1) % buffer->bufferSize;
        if (nextHead == buffer->tail) {
            return false; // Tampon dolu
        }

        buffer->buffer[buffer->head] = data[i];
        buffer->head = nextHead;
    }

    return true;
}

// Dairesel Tampondan Veri Okuma Fonksiyonu
uint32_t CircularBuffer_Read(CircularBuffer* buffer, uint8_t* data, uint32_t size) {
    if (buffer == NULL || data == NULL || size == 0) {
        return 0;
    }

    uint32_t readCount = 0;
    for (uint32_t i = 0; i < size; i++) {
        if (buffer->tail == buffer->head) {
            break; // Tampon boş
        }

        data[i] = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
        readCount++;
    }

    return readCount;
}