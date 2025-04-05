#ifndef CNT_MATH_H
#define CNT_MATH_H

#include <stdint.h>
#include <stdbool.h>

// Mutlak Değer Fonksiyonu (Tamsayı)
int32_t Math_Abs(int32_t num);

// Mutlak Değer Fonksiyonu (Kayan Nokta)
float Math_FAbs(float num);

// Üs Alma Fonksiyonu
float Math_Pow(float base, float exponent);

// Karekök Alma Fonksiyonu
float Math_Sqrt(float num);

// Sinüs Fonksiyonu
float Math_Sin(float angle);

// Kosinüs Fonksiyonu
float Math_Cos(float angle);

// Tanjant Fonksiyonu
float Math_Tan(float angle);

// Arksinüs Fonksiyonu
float Math_ASin(float value);

// Arkkosinüs Fonksiyonu
float Math_ACos(float value);

// Arktanjant Fonksiyonu
float Math_ATan(float value);

// Yuvarlama Fonksiyonu
int32_t Math_Round(float num);

// Yukarı Yuvarlama Fonksiyonu
int32_t Math_Ceil(float num);

// Aşağı Yuvarlama Fonksiyonu
int32_t Math_Floor(float num);

// Pi Sabiti
#define MATH_PI 3.14159265358979323846f

#endif