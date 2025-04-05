#ifndef CNT_MAP_H
#define CNT_MAP_H

#include <stdbool.h>
#include <stddef.h>

// Harita Öğesi Yapısı
typedef struct MapEntry {
    void* key;
    void* value;
    struct MapEntry* next;
} MapEntry;

// Harita Yapısı
typedef struct {
    MapEntry** buckets;
    size_t capacity;
    size_t size;
} Map;

// Harita Oluşturma Fonksiyonu
Map* create_map(size_t initial_capacity);

// Harita Öğesi Ekleme Fonksiyonu
void map_insert(Map* map, void* key, void* value, int (*compare)(void*, void*), unsigned long (*hash)(void*));

// Harita Öğesi Arama Fonksiyonu
void* map_find(Map* map, void* key, int (*compare)(void*, void*), unsigned long (*hash)(void*));

// Harita Öğesi Kaldırma Fonksiyonu
void map_remove(Map* map, void* key, int (*compare)(void*, void*), unsigned long (*hash)(void*));

// Harita Boyutu Fonksiyonu
size_t map_size(Map* map);

// Harita Boş Mu Fonksiyonu
bool map_is_empty(Map* map);

// Harita Temizleme Fonksiyonu
void map_clear(Map* map);

// Harita Serbest Bırakma Fonksiyonu
void free_map(Map* map);

#endif