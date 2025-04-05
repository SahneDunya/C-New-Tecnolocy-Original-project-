#ifndef CNT_TYPE_UTILS_H
#define CNT_TYPE_UTILS_H

#include <stdbool.h>
#include "cnt_type.h" // Type yapısı için

// İki tipin aynı olup olmadığını kontrol etme
bool types_are_equal(const Type* type1, const Type* type2);

// Bir tipin boyutunu alma (basit bir yaklaşım)
size_t get_type_size(const Type* type);

// Bir tipin adını alma
const char* get_type_name(const Type* type);

// Bir tipin temel türünü alma
BaseType get_base_type(const Type* type);

// Bir tipin pointer olup olmadığını kontrol etme
bool is_pointer_type(const Type* type);

// Bir tipin array olup olmadığını kontrol etme
bool is_array_type(const Type* type);

// Bir tipin sayısal (tam sayı veya kayan nokta) olup olmadığını kontrol etme
bool is_numeric_type(const Type* type);

// Bir tipin tam sayı olup olmadığını kontrol etme
bool is_integer_type(const Type* type);

// Bir tipin kayan nokta olup olmadığını kontrol etme
bool is_floating_point_type(const Type* type);

// Bir tipin boolean olup olmadığını kontrol etme
bool is_boolean_type(const Type* type);

// Bir tipin karakter olup olmadığını kontrol etme
bool is_character_type(const Type* type);

// Bir tipin string olup olmadığını kontrol etme
bool is_string_type(const Type* type);

// ... diğer yardımcı fonksiyonlar ...

#endif