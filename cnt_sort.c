#include <stdlib.h>
#include "cnt_sort.h"

// Hızlı Sıralama Yardımcı Fonksiyonları
static void swap(void** array, size_t i, size_t j) {
    void* temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

static size_t partition(void** array, size_t low, size_t high, CompareFunction compare) {
    void* pivot = array[high];
    size_t i = low - 1;

    for (size_t j = low; j < high; j++) {
        if (compare(array[j], pivot) <= 0) {
            i++;
            swap(array, i, j);
        }
    }

    swap(array, i + 1, high);
    return i + 1;
}

static void quick_sort_recursive(void** array, size_t low, size_t high, CompareFunction compare) {
    if (low < high) {
        size_t pivot_index = partition(array, low, high, compare);
        quick_sort_recursive(array, low, pivot_index - 1, compare);
        quick_sort_recursive(array, pivot_index + 1, high, compare);
    }
}

void quick_sort(void** array, size_t size, CompareFunction compare) {
    if (size > 1) {
        quick_sort_recursive(array, 0, size - 1, compare);
    }
}

// Birleştirme Sıralaması Yardımcı Fonksiyonları
static void merge(void** array, size_t left, size_t middle, size_t right, CompareFunction compare) {
    size_t left_size = middle - left + 1;
    size_t right_size = right - middle;

    void** left_array = (void**)malloc(left_size * sizeof(void*));
    void** right_array = (void**)malloc(right_size * sizeof(void*));

    for (size_t i = 0; i < left_size; i++) {
        left_array[i] = array[left + i];
    }

    for (size_t j = 0; j < right_size; j++) {
        right_array[j] = array[middle + 1 + j];
    }

    size_t i = 0, j = 0, k = left;

    while (i < left_size && j < right_size) {
        if (compare(left_array[i], right_array[j]) <= 0) {
            array[k++] = left_array[i++];
        } else {
            array[k++] = right_array[j++];
        }
    }

    while (i < left_size) {
        array[k++] = left_array[i++];
    }

    while (j < right_size) {
        array[k++] = right_array[j++];
    }

    free(left_array);
    free(right_array);
}

static void merge_sort_recursive(void** array, size_t left, size_t right, CompareFunction compare) {
    if (left < right) {
        size_t middle = left + (right - left) / 2;
        merge_sort_recursive(array, left, middle, compare);
        merge_sort_recursive(array, middle + 1, right, compare);
        merge(array, left, middle, right, compare);
    }
}

void merge_sort(void** array, size_t size, CompareFunction compare) {
    if (size > 1) {
        merge_sort_recursive(array, 0, size - 1, compare);
    }
}