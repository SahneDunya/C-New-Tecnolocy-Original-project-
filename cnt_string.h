#ifndef CNT_STRING_H
#define CNT_STRING_H

#include <stddef.h>
#include <stdbool.h>

// String Uzunluğu Fonksiyonu
size_t cnt_string_length(const char* str);

// String Kopyalama Fonksiyonu
char* cnt_string_copy(const char* str);

// String Birleştirme Fonksiyonu
char* cnt_string_concat(const char* str1, const char* str2);

// String Karşılaştırma Fonksiyonu
int cnt_string_compare(const char* str1, const char* str2);

// Alt String Arama Fonksiyonu
char* cnt_string_find(const char* str, const char* substr);

// String Bölme Fonksiyonu
char** cnt_string_split(const char* str, const char* delimiter, size_t* result_count);

// String'i Tam Sayıya Dönüştürme Fonksiyonu
int cnt_string_to_int(const char* str);

// String'i Ondalıklı Sayıya Dönüştürme Fonksiyonu
double cnt_string_to_double(const char* str);

// String'i Boole Değerine Dönüştürme Fonksiyonu
bool cnt_string_to_bool(const char* str);

// String'i Küçük Harfe Dönüştürme Fonksiyonu
char* cnt_string_to_lower(const char* str);

// String'i Büyük Harfe Dönüştürme Fonksiyonu
char* cnt_string_to_upper(const char* str);

// String'i Tersine Çevirme Fonksiyonu
char* cnt_string_reverse(const char* str);

// String'i Kırpma Fonksiyonu (Baş ve Sondaki Boşlukları Kaldırma)
char* cnt_string_trim(const char* str);

#endif