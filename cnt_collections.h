#ifndef CNT_COLLECTIONS_H
#define CNT_COLLECTIONS_H

#include <stddef.h>

// Dizi Oluşturma Fonksiyonu
int* cnt_create_array(size_t size);

// Dizi Serbest Bırakma Fonksiyonu
void cnt_free_array(int* array);

// Dizi Elemanı Ayarlama Fonksiyonu
void cnt_array_set(int* array, size_t index, int value);

// Dizi Elemanı Alma Fonksiyonu
int cnt_array_get(const int* array, size_t index);

#endif