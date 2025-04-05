#include "cnt_math.h"
#include <math.h>

// Mutlak Değer Fonksiyonu (Tamsayı)
int32_t Math_Abs(int32_t num) {
    return (num < 0) ? -num : num;
}

// Mutlak Değer Fonksiyonu (Kayan Nokta)
float Math_FAbs(float num) {
    return fabsf(num);
}

// Üs Alma Fonksiyonu
float Math_Pow(float base, float exponent) {
    return powf(base, exponent);
}

// Karekök Alma Fonksiyonu
float Math_Sqrt(float num) {
    return sqrtf(num);
}

// Sinüs Fonksiyonu
float Math_Sin(float angle) {
    return sinf(angle);
}

// Kosinüs Fonksiyonu
float Math_Cos(float angle) {
    return cosf(angle);
}

// Tanjant Fonksiyonu
float Math_Tan(float angle) {
    return tanf(angle);
}

// Arksinüs Fonksiyonu
float Math_ASin(float value) {
    return asinf(value);
}

// Arkkosinüs Fonksiyonu
float Math_ACos(float value) {
    return acosf(value);
}

// Arktanjant Fonksiyonu
float Math_ATan(float value) {
    return atanf(value);
}

// Yuvarlama Fonksiyonu
int32_t Math_Round(float num) {
    return (int32_t)roundf(num);
}

// Yukarı Yuvarlama Fonksiyonu
int32_t Math_Ceil(float num) {
    return (int32_t)ceilf(num);
}

// Aşağı Yuvarlama Fonksiyonu
int32_t Math_Floor(float num) {
    return (int32_t)floorf(num);
}