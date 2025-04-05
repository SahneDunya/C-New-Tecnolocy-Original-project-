#ifndef CNT_MATH_H
#define CNT_MATH_H

#include <stddef.h>

// Mutlak Değer Fonksiyonu
int abs_int(int value);
double abs_double(double value);

// Üs Alma Fonksiyonu
double pow_int(int base, int exponent);
double pow_double(double base, double exponent);

// Karekök Fonksiyonu
double sqrt_double(double value);

// Trigonometrik Fonksiyonlar
double sin_double(double angle);
double cos_double(double angle);
double tan_double(double angle);

// Rastgele Sayı Üretme Fonksiyonu
int rand_int(int min, int max);

#endif