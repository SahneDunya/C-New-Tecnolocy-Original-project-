#include <stdio.h>
#include <stdlib.h>
#include "cnt_core.h"

void cnt_exit(int status) {
    exit(status);
}

void* cnt_malloc(size_t size) {
    return malloc(size);
}

void cnt_free(void* ptr) {
    free(ptr);
}

void cnt_print_int(int value) {
    printf("%d", value);
}

void cnt_print_float(float value) {
    printf("%f", value);
}

void cnt_print_string(char* str) {
    printf("%s", str);
}