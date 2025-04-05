#include "cnt_io.h"
#include "cnt_core.h" // cnt_malloc için

int cnt_read_int() {
    int value;
    scanf("%d", &value);
    return value;
}

float cnt_read_float() {
    float value;
    scanf("%f", &value);
    return value;
}

char* cnt_read_string() {
    char buffer[1024];
    scanf("%s", buffer);
    char* str = (char*)cnt_malloc(strlen(buffer) + 1);
    strcpy(str, buffer);
    return str;
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

void cnt_print_char(char c) {
    printf("%c", c);
}

void cnt_print_newline() {
    printf("\n");
}