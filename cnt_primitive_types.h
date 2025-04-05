#ifndef CNT_PRIMITIVE_TYPES_H
#define CNT_PRIMITIVE_TYPES_H

#include <limits.h> // Standart limitler için
#include <float.h>  // Kayan nokta limitleri için
#include <stdbool.h>

// Temel Tam Sayı Tipleri ve Limitleri
typedef int cnt_int; // Varsayılan tam sayı tipi
typedef short cnt_short;
typedef long cnt_long;

#define CNT_INT_MIN INT_MIN
#define CNT_INT_MAX INT_MAX
#define CNT_SHORT_MIN SHRT_MIN
#define CNT_SHORT_MAX SHRT_MAX
#define CNT_LONG_MIN LONG_MIN
#define CNT_LONG_MAX LONG_MAX

// İşaretsiz Tam Sayı Tipleri ve Limitleri
typedef unsigned int cnt_uint;
typedef unsigned short cnt_ushort;
typedef unsigned long cnt_ulong;

#define CNT_UINT_MIN 0
#define CNT_UINT_MAX UINT_MAX
#define CNT_USHORT_MIN 0
#define CNT_USHORT_MAX USHRT_MAX
#define CNT_ULONG_MIN 0
#define CNT_ULONG_MAX ULONG_MAX

// Kayan Noktalı Tipler ve Limitleri
typedef float cnt_float;
typedef double cnt_double;

#define CNT_FLOAT_MIN FLT_MIN
#define CNT_FLOAT_MAX FLT_MAX
#define CNT_DOUBLE_MIN DBL_MIN
#define CNT_DOUBLE_MAX DBL_MAX

// Boolean Tipi
typedef bool cnt_bool;
#define CNT_TRUE true
#define CNT_FALSE false

// Karakter Tipi
typedef char cnt_char;
#define CNT_CHAR_MIN CHAR_MIN
#define CNT_CHAR_MAX CHAR_MAX

// String Tipi (Aslında birleşik bir tip olsa da, burada temel bir tanım verilebilir)
typedef char* cnt_string;

// Yardımcı Fonksiyon Prototipleri
bool cnt_is_valid_int(cnt_int value);
bool cnt_is_valid_float(cnt_float value);
// ... diğer tipler için doğrulama fonksiyonları ...

#endif