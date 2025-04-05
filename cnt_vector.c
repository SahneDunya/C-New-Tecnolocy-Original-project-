#include <stdio.h>
#include <stdlib.h>
#include "cnt_vector.h"

Vector* create_vector(size_t initial_capacity) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->data = (void**)malloc(initial_capacity * sizeof(void*));
    return vector;
}

void vector_push_back(Vector* vector, void* data) {
    if (vector->size == vector->capacity) {
        vector->capacity *= 2;
        vector->data = (void**)realloc(vector->data, vector->capacity * sizeof(void*));
    }

    vector->data[vector->size++] = data;
}

void* vector_pop_back(Vector* vector) {
    if (vector->size == 0) {
        return NULL;
    }

    return vector->data[--vector->size];
}

void* vector_get(Vector* vector, size_t index) {
    if (index >= vector->size) {
        return NULL;
    }

    return vector->data[index];
}

size_t vector_size(Vector* vector) {
    return vector->size;
}

size_t vector_capacity(Vector* vector) {
    return vector->capacity;
}

bool vector_is_empty(Vector* vector) {
    return vector->size == 0;
}

void vector_clear(Vector* vector) {
    vector->size = 0;
}

void free_vector(Vector* vector) {
    free(vector->data);
    free(vector);
}