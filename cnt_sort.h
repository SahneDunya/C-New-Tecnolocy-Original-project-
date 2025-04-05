#ifndef CNT_SORT_H
#define CNT_SORT_H

#include <stddef.h>

// Sıralama Fonksiyonu İşaretçisi
typedef int (*CompareFunction)(void*, void*);

// Hızlı Sıralama Fonksiyonu
void quick_sort(void** array, size_t size, CompareFunction compare);

// Birleştirme Sıralaması Fonksiyonu
void merge_sort(void** array, size_t size, CompareFunction compare);

#endif