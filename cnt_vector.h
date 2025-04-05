#ifndef CNT_VECTOR_H
#define CNT_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

// Vektör Yapısı
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} Vector;

// Vektör Oluşturma Fonksiyonu
Vector* create_vector(size_t initial_capacity);

// Vektör Öğesi Ekleme Fonksiyonu
void vector_push_back(Vector* vector, void* data);

// Vektör Öğesi Kaldırma Fonksiyonu
void* vector_pop_back(Vector* vector);

// Vektör Öğesine Erişim Fonksiyonu
void* vector_get(Vector* vector, size_t index);

// Vektör Boyutu Fonksiyonu
size_t vector_size(Vector* vector);

// Vektör Kapasitesi Fonksiyonu
size_t vector_capacity(Vector* vector);

// Vektör Boş Mu Fonksiyonu
bool vector_is_empty(Vector* vector);

// Vektör Temizleme Fonksiyonu
void vector_clear(Vector* vector);

// Vektör Serbest Bırakma Fonksiyonu
void free_vector(Vector* vector);

#endif