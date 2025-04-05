#include <math.h>
#include <stdlib.h>
#include "cnt_math.h"

// Mutlak Değer Fonksiyonları
int abs_int(int value) {
    return (value < 0) ? -value : value;
}

double abs_double(double value) {
    return fabs(value);
}

// Üs Alma Fonksiyonları
double pow_int(int base, int exponent) {
    double result = 1.0;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

double pow_double(double base, double exponent) {
    return pow(base, exponent);
}

// Karekök Fonksiyonu
double sqrt_double(double value) {
    return sqrt(value);
}

// Trigonometrik Fonksiyonlar
double sin_double(double angle) {
    return sin(angle);
}

double cos_double(double angle) {
    return cos(angle);
}

double tan_double(double angle) {
    return tan(angle);
}

// Rastgele Sayı Üretme Fonksiyonu
int rand_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}