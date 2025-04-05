#ifndef CNT_TIME_H
#define CNT_TIME_H

#include <stdint.h>
#include <time.h>

// Geçerli Zamanı Alma Fonksiyonu
uint64_t get_current_time();

// Zamanı Dizeye Dönüştürme Fonksiyonu
char* format_time(uint64_t timestamp, const char* format);

// Zamanı Dizeden Dönüştürme Fonksiyonu
uint64_t parse_time(const char* time_string, const char* format);

// Uyuma Fonksiyonu
void sleep_ms(uint32_t milliseconds);

#endif