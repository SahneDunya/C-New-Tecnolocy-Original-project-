#include "cnt_reflection.h"
#include <stdlib.h>
#include <string.h>

// Yansıma Oluşturma Fonksiyonu
Reflection* reflection_create() {
    Reflection* reflection = (Reflection*)malloc(sizeof(Reflection));
    if (reflection == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    reflection->types = NULL;
    reflection->type_count = 0;
    reflection->fields = NULL;
    reflection->field_count = 0;
    reflection->functions = NULL;
    reflection->function_count = 0;
    return reflection;
}

// Tür Bilgisi Ekleme Fonksiyonu
bool reflection_add_type(Reflection* reflection, TypeInfo* type) {
    reflection->types = (TypeInfo**)realloc(reflection->types, sizeof(TypeInfo*) * (reflection->type_count + 1));
    if (reflection->types == NULL) {
        return false; // Bellek ayırma hatası
    }
    reflection->types[reflection->type_count] = type;
    reflection->type_count++;
    return true;
}

// Alan Bilgisi Ekleme Fonksiyonu
bool reflection_add_field(Reflection* reflection, FieldInfo* field) {
    reflection->fields = (FieldInfo**)realloc(reflection->fields, sizeof(FieldInfo*) * (reflection->field_count + 1));
    if (reflection->fields == NULL) {
        return false; // Bellek ayırma hatası
    }
    reflection->fields[reflection->field_count] = field;
    reflection->field_count++;
    return true;
}

// Fonksiyon Bilgisi Ekleme Fonksiyonu
bool reflection_add_function(Reflection* reflection, FunctionInfo* function) {
    reflection->functions = (FunctionInfo**)realloc(reflection->functions, sizeof(FunctionInfo*) * (reflection->function_count + 1));
    if (reflection->functions == NULL) {
        return false; // Bellek ayırma hatası
    }
    reflection->functions[reflection->function_count] = function;
    reflection->function_count++;
    return true;
}

// Tür Bilgisi Alma Fonksiyonu
TypeInfo* reflection_get_type(Reflection* reflection, const char* name) {
    for (size_t i = 0; i < reflection->type_count; i++) {
        if (strcmp(reflection->types[i]->name, name) == 0) {
            return reflection->types[i];
        }
    }
    return NULL; // Tür bulunamadı
}

// Alan Bilgisi Alma Fonksiyonu
FieldInfo* reflection_get_field(Reflection* reflection, const char* name) {
    for (size_t i = 0; i < reflection->field_count; i++) {
        if (strcmp(reflection->fields[i]->name, name) == 0) {
            return reflection->fields[i];
        }
    }
    return NULL; // Alan bulunamadı
}

// Fonksiyon Bilgisi Alma Fonksiyonu
FunctionInfo* reflection_get_function(Reflection* reflection, const char* name) {
    for (size_t i = 0; i < reflection->function_count; i++) {
        if (strcmp(reflection->functions[i]->name, name) == 0) {
            return reflection->functions[i];
        }
    }
    return NULL; // Fonksiyon bulunamadı
}

// Yansıma Serbest Bırakma Fonksiyonu
void reflection_free(Reflection* reflection) {
    free(reflection->types);
    free(reflection->fields);
    free(reflection->functions);
    free(reflection);
}