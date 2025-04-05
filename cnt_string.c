#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "cnt_string.h"

size_t cnt_string_length(const char* str) {
    return strlen(str);
}

char* cnt_string_copy(const char* str) {
    char* copy = (char*)malloc(strlen(str) + 1);
    strcpy(copy, str);
    return copy;
}

char* cnt_string_concat(const char* str1, const char* str2) {
    char* result = (char*)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

int cnt_string_compare(const char* str1, const char* str2) {
    return strcmp(str1, str2);
}

char* cnt_string_find(const char* str, const char* substr) {
    return strstr(str, substr);
}

char** cnt_string_split(const char* str, const char* delimiter, size_t* result_count) {
    char* copy = cnt_string_copy(str);
    char* token = strtok(copy, delimiter);
    char** result = NULL;
    *result_count = 0;

    while (token != NULL) {
        result = (char**)realloc(result, (*result_count + 1) * sizeof(char*));
        result[*result_count] = cnt_string_copy(token);
        (*result_count)++;
        token = strtok(NULL, delimiter);
    }

    free(copy);
    return result;
}

int cnt_string_to_int(const char* str) {
    return atoi(str);
}

double cnt_string_to_double(const char* str) {
    return atof(str);
}

bool cnt_string_to_bool(const char* str) {
    if (strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0) {
        return true;
    } else if (strcasecmp(str, "false") == 0 || strcmp(str, "0") == 0) {
        return false;
    } else {
        return false; // Varsayılan olarak false döndür
    }
}

char* cnt_string_to_lower(const char* str) {
    char* result = cnt_string_copy(str);
    for (char* p = result; *p; p++) {
        *p = tolower(*p);
    }
    return result;
}

char* cnt_string_to_upper(const char* str) {
    char* result = cnt_string_copy(str);
    for (char* p = result; *p; p++) {
        *p = toupper(*p);
    }
    return result;
}

char* cnt_string_reverse(const char* str) {
    size_t length = strlen(str);
    char* result = (char*)malloc(length + 1);
    for (size_t i = 0; i < length; i++) {
        result[i] = str[length - 1 - i];
    }
    result[length] = '\0';
    return result;
}

char* cnt_string_trim(const char* str) {
    const char* start = str;
    while (isspace(*start)) start++;
    if (*start == '\0') {
        return cnt_string_copy(""); // Tüm karakterler boşluksa boş string döndür
    }
    const char* end = str + strlen(str) - 1;
    while (end > start && isspace(*end)) end--;
    size_t length = end - start + 1;
    char* result = (char*)malloc(length + 1);
    strncpy(result, start, length);
    result[length] = '\0';
    return result;
}