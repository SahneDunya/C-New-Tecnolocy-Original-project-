#ifndef CNT_SEARCH_H
#define CNT_SEARCH_H

#include <stdbool.h>
#include <stddef.h>

// Arama Fonksiyonu İşaretçisi
typedef int (*CompareFunction)(void*, void*);

// Doğrusal Arama Fonksiyonu
void* linear_search(void** array, size_t size, void* key, CompareFunction compare);

// İkili Arama Fonksiyonu
void* binary_search(void** array, size_t size, void* key, CompareFunction compare);

#endif