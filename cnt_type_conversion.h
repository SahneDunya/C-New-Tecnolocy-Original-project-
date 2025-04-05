#ifndef CNT_TYPE_CONVERSION_H
#define CNT_TYPE_CONVERSION_H

#include <stdbool.h>
#include "cnt_type.h" // Type yapısı için

// Dönüşüm Türleri (isteğe bağlı)
typedef enum {
    CONVERSION_IMPLICIT,
    CONVERSION_EXPLICIT
} ConversionType;

// Temel Tipler Arası Dönüşüm Fonksiyon Prototipleri
bool can_convert_to(const Type* from_type, const Type* to_type, ConversionType type);
// Temel tipten temel tipe dönüşüm fonksiyonları
// (Geri dönüş değeri, dönüşüm başarılıysa true, başarısızsa false olabilir.
// Dönüştürülmüş değer, bir çıktı parametresi aracılığıyla döndürülebilir.)
bool convert_int_to_float(int from_value, float* to_value);
bool convert_float_to_int(float from_value, int* to_value);
bool convert_int_to_bool(int from_value, bool* to_value);
bool convert_bool_to_int(bool from_value, int* to_value);
bool convert_char_to_int(char from_value, int* to_value);
bool convert_int_to_char(int from_value, char* to_value);
// ... diğer temel tip dönüşümleri ...

// Birleşik Tipler Arası Dönüşüm (şimdilik temel)
// (Gerekirse dizi, yapı vb. dönüşümleri için prototipler eklenebilir)

// Modern Özellikler İçin (Örneğin, String'e veya String'den Dönüşüm)
bool convert_any_to_string(const void* from_value, const Type* from_type, char** to_string);
bool convert_string_to_any(const char* from_string, const Type* to_type, void* to_value);

#endif