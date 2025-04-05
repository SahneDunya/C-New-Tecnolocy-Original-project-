#ifndef CNT_SET_H
#define CNT_SET_H

#include <stdbool.h>
#include <stddef.h>

// Küme Yapısı
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
    int (*compare)(void*, void*);
    unsigned long (*hash)(void*);
} Set;

// Küme Oluşturma Fonksiyonu
Set* create_set(size_t initial_capacity, int (*compare)(void*, void*), unsigned long (*hash)(void*));

// Küme Öğesi Ekleme Fonksiyonu
bool set_insert(Set* set, void* data);

// Küme Öğesi Arama Fonksiyonu
bool set_contains(Set* set, void* data);

// Küme Öğesi Kaldırma Fonksiyonu
bool set_remove(Set* set, void* data);

// Küme Boyutu Fonksiyonu
size_t set_size(Set* set);

// Küme Boş Mu Fonksiyonu
bool set_is_empty(Set* set);

// Küme Temizleme Fonksiyonu
void set_clear(Set* set);

// Küme Serbest Bırakma Fonksiyonu
void free_set(Set* set);

#endif