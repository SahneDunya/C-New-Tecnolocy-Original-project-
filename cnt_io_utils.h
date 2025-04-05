#ifndef CNT_IO_UTILS_H
#define CNT_IO_UTILS_H

#include <stdbool.h>
#include <stdlib.h> // strtol, strtod için

// String'i tamsayıya (int) dönüştürme fonksiyonu
// Başarılı dönüşümde true, başarısızlıkta false döner.
// Başarısızlık durumunda errno global değişkeni ayarlanır.
bool cnt_str_to_int(const char* str, int* out_value);

// String'i ondalıklı sayıya (double) dönüştürme fonksiyonu
// Başarılı dönüşümde true, başarısızlıkta false döner.
// Başarısızlık durumunda errno global değişkeni ayarlanır.
bool cnt_str_to_double(const char* str, double* out_value);

// Bir string'in başındaki ve sonundaki boşlukları (whitespace) temizleyen fonksiyon
// Yeni bir string döndürür, çağıran tarafın belleği serbest bırakması gerekir.
char* cnt_str_trim(const char* str);

#endif