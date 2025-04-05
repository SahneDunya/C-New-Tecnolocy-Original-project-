#include "cnt_io_utils.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>

bool cnt_str_to_int(const char* str, int* out_value) {
    if (str == NULL || out_value == NULL) {
        errno = EINVAL; // Geçersiz argüman
        return false;
    }
    char* endptr;
    long val = strtol(str, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        return false; // Aralık hatası veya başka bir hata
    }
    if (endptr == str || *endptr != '\0') {
        return false; // Dönüşüm yapılamadı veya string'in sonunda geçersiz karakterler var
    }
    if (val > INT_MAX || val < INT_MIN) {
        errno = ERANGE; // Değer int aralığının dışında
        return false;
    }
    *out_value = (int)val;
    return true;
}

bool cnt_str_to_double(const char* str, double* out_value) {
    if (str == NULL || out_value == NULL) {
        errno = EINVAL; // Geçersiz argüman
        return false;
    }
    char* endptr;
    double val = strtod(str, &endptr);
    if (errno == ERANGE) {
        return false; // Aralık hatası
    }
    if (endptr == str || *endptr != '\0') {
        return false; // Dönüşüm yapılamadı veya string'in sonunda geçersiz karakterler var
    }
    *out_value = val;
    return true;
}

char* cnt_str_trim(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    size_t start = 0;
    size_t end = len - 1;

    while (start <= end && isspace((unsigned char)str[start])) {
        start++;
    }
    while (end >= start && isspace((unsigned char)str[end])) {
        end--;
    }

    if (start > end) {
        char* trimmed = (char*)malloc(1);
        if (trimmed) {
            trimmed[0] = '\0';
        }
        return trimmed;
    }

    size_t trimmed_len = end - start + 1;
    char* trimmed = (char*)malloc(trimmed_len + 1);
    if (trimmed == NULL) {
        perror("Bellek ayırma hatası");
        return NULL;
    }
    strncpy(trimmed + 0, str + start, trimmed_len);
    trimmed[trimmed_len] = '\0';
    return trimmed;
}