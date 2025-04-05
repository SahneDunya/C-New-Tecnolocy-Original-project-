#include <stdio.h>
#include <stdlib.h>
#include "cnt_set.h"

Set* create_set(size_t initial_capacity, int (*compare)(void*, void*), unsigned long (*hash)(void*)) {
    Set* set = (Set*)malloc(sizeof(Set));
    set->size = 0;
    set->capacity = initial_capacity;
    set->data = (void**)malloc(initial_capacity * sizeof(void*));
    set->compare = compare;
    set->hash = hash;
    return set;
}

bool set_insert(Set* set, void* data) {
    if (set_contains(set, data)) {
        return false;
    }

    if (set->size == set->capacity) {
        set->capacity *= 2;
        set->data = (void**)realloc(set->data, set->capacity * sizeof(void*));
    }

    set->data[set->size++] = data;
    return true;
}

bool set_contains(Set* set, void* data) {
    for (size_t i = 0; i < set->size; i++) {
        if (set->compare(set->data[i], data) == 0) {
            return true;
        }
    }
    return false;
}

bool set_remove(Set* set, void* data) {
    for (size_t i = 0; i < set->size; i++) {
        if (set->compare(set->data[i], data) == 0) {
            set->data[i] = set->data[set->size - 1];
            set->size--;
            return true;
        }
    }
    return false;
}

size_t set_size(Set* set) {
    return set->size;
}

bool set_is_empty(Set* set) {
    return set->size == 0;
}

void set_clear(Set* set) {
    set->size = 0;
}

void free_set(Set* set) {
    free(set->data);
    free(set);
}