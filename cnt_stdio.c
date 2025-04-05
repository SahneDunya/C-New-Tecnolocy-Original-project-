#include "cnt_stdio.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Standart çıktı fonksiyonları

void cnt_println(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void cnt_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Standart girdi fonksiyonları

char* cnt_read_line() {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, stdin);
    if (read != -1) {
        // getline fonksiyonu satır sonu karakterini de okur, onu kaldıralım
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        return line;
    } else {
        free(line);
        return NULL; // EOF veya hata durumunda NULL döndürülür
    }
}

int cnt_scanf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vscanf(format, args);
    va_end(args);
    return result;
}

// Standart hata çıktısı fonksiyonu

void cnt_eprintln(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}