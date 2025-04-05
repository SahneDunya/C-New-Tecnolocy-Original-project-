#include <stdio.h>
#include <stdlib.h>
#include "cnt_collections.h"

int* cnt_create_array(size_t size) {
    if (size == 0) {
        return NULL;
    }

    int* array = (int*)malloc(size * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Hata: Bellek ayırma başarısız.\n");
        return NULL;
    }

    return array;
}

void cnt_free_array(int* array) {
    if (array != NULL) {
        free(array);
    }
}

void cnt_array_set(int* array, size_t index, int value) {
    if (array == NULL) {
        fprintf(stderr, "Hata: Dizi boş.\n");
        return;
    }

    array[index] = value;
}

int cnt_array_get(const int* array, size_t index) {
    if (array == NULL) {
        fprintf(stderr, "Hata: Dizi boş.\n");
        return 0; // Hata durumunda varsayılan değer
    }

    return array[index];
}