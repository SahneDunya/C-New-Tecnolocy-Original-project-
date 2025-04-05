#ifndef CNT_SORT_H
#define CNT_SORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Kabarcık Sıralama Fonksiyonu
void BubbleSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*));

// Ekleme Sıralama Fonksiyonu
void InsertionSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*));

// Seçmeli Sıralama Fonksiyonu
void SelectionSort(void* arr, uint32_t size, size_t elementSize, int (*compare)(const void*, const void*));

#endif