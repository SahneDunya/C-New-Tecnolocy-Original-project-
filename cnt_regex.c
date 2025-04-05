#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "cnt_regex.h"

// Düzenli İfade Derleme Fonksiyonu
Regex* regex_compile(const char* pattern) {
    Regex* regex = (Regex*)malloc(sizeof(Regex));
    int result = regcomp(&regex->regex, pattern, REG_EXTENDED);
    if (result != 0) {
        free(regex);
        return NULL;
    }
    return regex;
}

// Düzenli İfade Eşleştirme Fonksiyonu
bool regex_match(Regex* regex, const char* text) {
    return regexec(&regex->regex, text, 0, NULL, 0) == 0;
}

// Düzenli İfade Arama Fonksiyonu
char* regex_search(Regex* regex, const char* text) {
    regmatch_t match;
    if (regexec(&regex->regex, text, 1, &match, 0) == 0) {
        char* result = (char*)malloc(match.rm_eo - match.rm_so + 1);
        strncpy(result, text + match.rm_so, match.rm_eo - match.rm_so);
        result[match.rm_eo - match.rm_so] = '\0';
        return result;
    }
    return NULL;
}

// Düzenli İfade Serbest Bırakma Fonksiyonu
void free_regex(Regex* regex) {
    regfree(&regex->regex);
    free(regex);
}