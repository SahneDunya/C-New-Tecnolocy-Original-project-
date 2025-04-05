#ifndef CNT_STRING_H
#define CNT_STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Dize Uzunluğunu Bulma Fonksiyonu
uint32_t String_Length(const char* str);

// Dize Kopyalama Fonksiyonu
char* String_Copy(char* dest, const char* src);

// Dize Birleştirme Fonksiyonu
char* String_Concatenate(char* dest, const char* src);

// Dize Karşılaştırma Fonksiyonu
int String_Compare(const char* str1, const char* str2);

// Dize Alt Dize Arama Fonksiyonu
char* String_Find(const char* str, const char* subStr);

// Dizeyi Tamsayıya Dönüştürme Fonksiyonu
int32_t String_ToInteger(const char* str);

// Tamsayıyı Dizeye Dönüştürme Fonksiyonu
char* Integer_ToString(int32_t num, char* str, uint32_t size);

#endif