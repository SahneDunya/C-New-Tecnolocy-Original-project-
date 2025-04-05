#ifndef CNT_REGEX_H
#define CNT_REGEX_H

#include <stdbool.h>
#include <stddef.h>

// Düzenli İfade Yapısı
typedef struct {
    regex_t regex;
} Regex;

// Düzenli İfade Derleme Fonksiyonu
Regex* regex_compile(const char* pattern);

// Düzenli İfade Eşleştirme Fonksiyonu
bool regex_match(Regex* regex, const char* text);

// Düzenli İfade Arama Fonksiyonu
char* regex_search(Regex* regex, const char* text);

// Düzenli İfade Serbest Bırakma Fonksiyonu
void free_regex(Regex* regex);

#endif