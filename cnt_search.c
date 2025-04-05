#include "cnt_search.h"

// Doğrusal Arama
void* linear_search(void** array, size_t size, void* key, CompareFunction compare) {
    for (size_t i = 0; i < size; i++) {
        if (compare(array[i], key) == 0) {
            return array[i];
        }
    }
    return NULL;
}

// İkili Arama
void* binary_search(void** array, size_t size, void* key, CompareFunction compare) {
    size_t low = 0;
    size_t high = size - 1;

    while (low <= high) {
        size_t mid = low + (high - low) / 2;
        int cmp = compare(array[mid], key);

        if (cmp == 0) {
            return array[mid];
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return NULL;
}