#ifndef CNT_REFLECTION_H
#define CNT_REFLECTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Tür Bilgisi Yapısı
typedef struct TypeInfo {
    const char* name;
    size_t size;
    // ... diğer tür bilgileri ...
} TypeInfo;

// Alan Bilgisi Yapısı
typedef struct FieldInfo {
    const char* name;
    TypeInfo* type;
    size_t offset;
    // ... diğer alan bilgileri ...
} FieldInfo;

// Fonksiyon Bilgisi Yapısı
typedef struct FunctionInfo {
    const char* name;
    TypeInfo* return_type;
    TypeInfo** parameter_types;
    size_t parameter_count;
    // ... diğer fonksiyon bilgileri ...
} FunctionInfo;

// Yansıma Yapısı
typedef struct Reflection {
    TypeInfo** types;
    size_t type_count;
    FieldInfo** fields;
    size_t field_count;
    FunctionInfo** functions;
    size_t function_count;
} Reflection;

// Yansıma Oluşturma Fonksiyonu
Reflection* reflection_create();

// Tür Bilgisi Ekleme Fonksiyonu
bool reflection_add_type(Reflection* reflection, TypeInfo* type);

// Alan Bilgisi Ekleme Fonksiyonu
bool reflection_add_field(Reflection* reflection, FieldInfo* field);

// Fonksiyon Bilgisi Ekleme Fonksiyonu
bool reflection_add_function(Reflection* reflection, FunctionInfo* function);

// Tür Bilgisi Alma Fonksiyonu
TypeInfo* reflection_get_type(Reflection* reflection, const char* name);

// Alan Bilgisi Alma Fonksiyonu
FieldInfo* reflection_get_field(Reflection* reflection, const char* name);

// Fonksiyon Bilgisi Alma Fonksiyonu
FunctionInfo* reflection_get_function(Reflection* reflection, const char* name);

// Yansıma Serbest Bırakma Fonksiyonu
void reflection_free(Reflection* reflection);

#endif