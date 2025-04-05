#include "cnt_string.h"

// Dize Uzunluğunu Bulma Fonksiyonu
uint32_t String_Length(const char* str) {
    if (str == NULL) {
        return 0;
    }

    uint32_t length = 0;
    while (str[length] != '\0') {
        length++;
    }

    return length;
}

// Dize Kopyalama Fonksiyonu
char* String_Copy(char* dest, const char* src) {
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    uint32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';

    return dest;
}

// Dize Birleştirme Fonksiyonu
char* String_Concatenate(char* dest, const char* src) {
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    uint32_t destLength = String_Length(dest);
    uint32_t i = 0;
    while (src[i] != '\0') {
        dest[destLength + i] = src[i];
        i++;
    }
    dest[destLength + i] = '\0';

    return dest;
}

// Dize Karşılaştırma Fonksiyonu
int String_Compare(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) {
        return 0;
    }

    uint32_t i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] < str2[i]) {
            return -1;
        } else if (str1[i] > str2[i]) {
            return 1;
        }
        i++;
    }

    if (str1[i] == '\0' && str2[i] == '\0') {
        return 0;
    } else if (str1[i] == '\0') {
        return -1;
    } else {
        return 1;
    }
}

// Dize Alt Dize Arama Fonksiyonu
char* String_Find(const char* str, const char* subStr) {
    if (str == NULL || subStr == NULL) {
        return NULL;
    }

    uint32_t strLength = String_Length(str);
    uint32_t subStrLength = String_Length(subStr);

    if (subStrLength > strLength) {
        return NULL;
    }

    for (uint32_t i = 0; i <= strLength - subStrLength; i++) {
        uint32_t j;
        for (j = 0; j < subStrLength; j++) {
            if (str[i + j] != subStr[j]) {
                break;
            }
        }
        if (j == subStrLength) {
            return (char*)str + i;
        }
    }

    return NULL;
}

// Dizeyi Tamsayıya Dönüştürme Fonksiyonu
int32_t String_ToInteger(const char* str) {
    if (str == NULL) {
        return 0;
    }

    int32_t result = 0;
    int32_t sign = 1;
    uint32_t i = 0;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
            i++;
        } else {
            break;
        }
    }

    return result * sign;
}

// Tamsayıyı Dizeye Dönüştürme Fonksiyonu
char* Integer_ToString(int32_t num, char* str, uint32_t size) {
    if (str == NULL || size == 0) {
        return NULL;
    }

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int32_t sign = 1;
    if (num < 0) {
        sign = -1;
        num = -num;
    }

    uint32_t i = 0;
    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (sign == -1) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Diziyi ters çevir
    for (uint32_t j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }

    return str;
}