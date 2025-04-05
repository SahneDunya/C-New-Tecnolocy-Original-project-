#include "cnt_type_utils.h"
#include "cnt_primitive_types.h" // Boyut bilgileri için (örneğin, sizeof)
#include <string.h>
#include <stddef.h> // size_t için

bool types_are_equal(const Type* type1, const Type* type2) {
    if (type1 == NULL || type2 == NULL) {
        return false;
    }
    if (type1->base_type != type2->base_type) {
        return false;
    }
    if (type1->name != NULL && type2->name != NULL) {
        return strcmp(type1->name, type2->name) == 0;
    }
    return true; // İsim yoksa temel türlerin eşit olduğunu varsayalım (dikkatli olunmalı)
}

size_t get_type_size(const Type* type) {
    if (type == NULL) {
        return 0;
    }
    switch (type->base_type) {
        case TYPE_INT:
            return sizeof(cnt_int);
        case TYPE_FLOAT:
            return sizeof(cnt_float);
        case TYPE_BOOL:
            return sizeof(cnt_bool);
        case TYPE_CHAR:
            return sizeof(cnt_char);
        case TYPE_POINTER:
            return sizeof(void*); // Pointer boyutu platforma göre değişebilir
        case TYPE_ARRAY:
            if (type->details.array.element_type != NULL) {
                return get_type_size(type->details.array.element_type) * type->details.array.size;
            }
            return 0;
        // Diğer tipler için boyut bilgisi eklenebilir (örneğin, struct, union)
        default:
            return 0; // Bilinmeyen veya boyutu belirlenemeyen tip
    }
}

const char* get_type_name(const Type* type) {
    if (type != NULL && type->name != NULL) {
        return type->name;
    }
    return "unknown";
}

BaseType get_base_type(const Type* type) {
    if (type != NULL) {
        return type->base_type;
    }
    return TYPE_VOID; // Veya başka bir varsayılan değer
}

bool is_pointer_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_POINTER;
}

bool is_array_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_ARRAY;
}

bool is_numeric_type(const Type* type) {
    if (type == NULL) return false;
    return is_integer_type(type) || is_floating_point_type(type);
}

bool is_integer_type(const Type* type) {
    if (type == NULL) return false;
    return type->base_type == TYPE_INT || type->base_type == TYPE_CHAR; // char da bir tam sayı türüdür
}

bool is_floating_point_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_FLOAT;
}

bool is_boolean_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_BOOL;
}

bool is_character_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_CHAR;
}

bool is_string_type(const Type* type) {
    return type != NULL && type->base_type == TYPE_STRING;
}

// ... diğer yardımcı fonksiyonların implementasyonları ...