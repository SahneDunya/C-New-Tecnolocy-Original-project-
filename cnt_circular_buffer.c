#include "cnt_circular_buffer.h"

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

// Dairesel Tampondaki Veri Sayısını Alma Fonksiyonu
uint32_t CircularBuffer_Available(CircularBuffer* buffer) {
    if (buffer == NULL) {
        return 0;
    }

    uint32_t available = 0;
    if (buffer->head >= buffer->tail) {
        available = buffer->head - buffer->tail;
    } else {
        available = buffer->bufferSize - buffer->tail + buffer->head;
    }

    return available;
}

// Dairesel Tamponun Boş Olup Olmadığını Kontrol Etme Fonksiyonu
bool CircularBuffer_IsEmpty(CircularBuffer* buffer) {
    if (buffer == NULL) {
        return true;
    }

    return buffer->head == buffer->tail;
}

// Dairesel Tamponun Dolu Olup Olmadığını Kontrol Etme Fonksiyonu
bool CircularBuffer_IsFull(CircularBuffer* buffer) {
    if (buffer == NULL) {
        return false;
    }

    return (buffer->head + 1) % buffer->bufferSize == buffer->tail;
}

// Dairesel Tamponu Sıfırlama Fonksiyonu
void CircularBuffer_Reset(CircularBuffer* buffer) {
    if (buffer != NULL) {
        buffer->head = 0;
        buffer->tail = 0;
    }
}