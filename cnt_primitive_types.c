#include "cnt_primitive_types.h"
#include <stdio.h>
#include <math.h> // Kayan nokta kontrolleri için

bool cnt_is_valid_int(cnt_int value) {
    // Şu anda tüm int değerlerini geçerli kabul ediyoruz.
    // İstenirse belirli aralık kontrolleri eklenebilir.
    return true;
}

bool cnt_is_valid_float(cnt_float value) {
    // NaN (Not a Number) ve sonsuz değerleri geçersiz kabul edebiliriz.
    return !isnan(value) && !isinf(value);
}

 bool cnt_is_valid_char(cnt_char value) {
 
 return true;
}