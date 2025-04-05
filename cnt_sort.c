#include "cnt_sort.h"
#include <string.h>

// Kabarcık Sıralama Fonksiyonu
void BubbleSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*)) {
    if (arr == NULL || size <= 1) {
        return;
    }

    uint8_t temp[elementSize];
    for (uint32_t i = 0; i < size - 1; i++) {
        for (uint32_t j = 0; j < size - i - 1; j++) {
            if (compare((uint8_t*)arr + j * elementSize, (uint8_t*)arr + (j + 1) * elementSize) > 0) {
                memcpy(temp, (uint8_t*)arr + j * elementSize, elementSize);
                memcpy((uint8_t*)arr + j * elementSize, (uint8_t*)arr + (j + 1) * elementSize, elementSize);
                memcpy((uint8_t*)arr + (j + 1) * elementSize, temp, elementSize);
            }
        }
    }
}

// Ekleme Sıralama Fonksiyonu
void InsertionSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*)) {
    if (arr == NULL || size <= 1) {
        return;
    }

    uint8_t temp[elementSize];
    for (uint32_t i = 1; i < size; i++) {
        memcpy(temp, (uint8_t*)arr + i * elementSize, elementSize);
        int j = i - 1;
        while (j >= 0 && compare((uint8_t*)arr + j * elementSize, temp) > 0) {
            memcpy((uint8_t*)arr + (j + 1) * elementSize, (uint8_t*)arr + j * elementSize, elementSize);
            j--;
        }
        memcpy((uint8_t*)arr + (j + 1) * elementSize, temp, elementSize);
    }
}

// Seçmeli Sıralama Fonksiyonu
void SelectionSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*)) {
    if (arr == NULL || size <= 1) {
        return;
    }

    uint8_t temp[elementSize];
    for (uint32_t i = 0; i < size - 1; i++) {
        uint32_t minIndex = i;
        for (uint32_t j = i + 1; j < size; j++) {
            if (compare((uint8_t*)arr + j * elementSize, (uint8_t*)arr + minIndex * elementSize) < 0) {
                minIndex = j;
            }
        }
        memcpy(temp, (uint8_t*)arr + i * elementSize, elementSize);
        memcpy((uint8_t*)arr + i * elementSize, (uint8_t*)arr + minIndex * elementSize, elementSize);
        memcpy((uint8_t*)arr + minIndex * elementSize, temp, elementSize);
    }
}