#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_time.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Geçerli Zamanı Alma Fonksiyonu
uint64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

// Zamanı Dizeye Dönüştürme Fonksiyonu
char* format_time(uint64_t timestamp, const char* format) {
    time_t time = (time_t)(timestamp / 1000);
    struct tm* time_info = localtime(&time);
    char* buffer = (char*)malloc(128 * sizeof(char));
    strftime(buffer, 128, format, time_info);
    return buffer;
}

// Zamanı Dizeden Dönüştürme Fonksiyonu
uint64_t parse_time(const char* time_string, const char* format) {
    struct tm time_info;
    strptime(time_string, format, &time_info);
    time_t time = mktime(&time_info);
    return (uint64_t)(time) * 1000;
}

// Uyuma Fonksiyonu
void sleep_ms(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}