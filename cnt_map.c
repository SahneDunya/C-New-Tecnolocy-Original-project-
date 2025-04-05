#include <stdio.h>
#include <stdlib.h>
#include "cnt_map.h"

Map* create_map(size_t initial_capacity) {
    Map* map = (Map*)malloc(sizeof(Map));
    map->capacity = initial_capacity;
    map->size = 0;
    map->buckets = (MapEntry**)calloc(map->capacity, sizeof(MapEntry*));
    return map;
}

void map_insert(Map* map, void* key, void* value, int (*compare)(void*, void*), unsigned long (*hash)(void*)) {
    unsigned long bucket_index = hash(key) % map->capacity;
    MapEntry* entry = map->buckets[bucket_index];

    while (entry != NULL) {
        if (compare(entry->key, key) == 0) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    MapEntry* new_entry = (MapEntry*)malloc(sizeof(MapEntry));
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = map->buckets[bucket_index];
    map->buckets[bucket_index] = new_entry;
    map->size++;
}

void* map_find(Map* map, void* key, int (*compare)(void*, void*), unsigned long (*hash)(void*)) {
    unsigned long bucket_index = hash(key) % map->capacity;
    MapEntry* entry = map->buckets[bucket_index];

    while (entry != NULL) {
        if (compare(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

void map_remove(Map* map, void* key, int (*compare)(void*, void*), unsigned long (*hash)(void*)) {
    unsigned long bucket_index = hash(key) % map->capacity;
    MapEntry* entry = map->buckets[bucket_index];
    MapEntry* prev = NULL;

    while (entry != NULL) {
        if (compare(entry->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[bucket_index] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry);
            map->size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

size_t map_size(Map* map) {
    return map->size;
}

bool map_is_empty(Map* map) {
    return map->size == 0;
}

void map_clear(Map* map) {
    for (size_t i = 0; i < map->capacity; i++) {
        MapEntry* entry = map->buckets[i];
        while (entry != NULL) {
            MapEntry* next = entry->next;
            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

void free_map(Map* map) {
    map_clear(map);
    free(map->buckets);
    free(map);
}