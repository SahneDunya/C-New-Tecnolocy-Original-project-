#include "cnt_type_conversion.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool can_convert_to(const Type* from_type, const Type* to_type, ConversionType type) {
    if (types_are_compatible(from_type, to_type)) {
        return true;
    }

    if (from_type->base_type == TYPE_INT) {
        if (to_type->base_type == TYPE_FLOAT) return true;
        if (to_type->base_type == TYPE_BOOL) return true;
        if (to_type->base_type == TYPE_CHAR) return true;
    } else if (from_type->base_type == TYPE_FLOAT) {
        if (to_type->base_type == TYPE_INT && type == CONVERSION_EXPLICIT) return true; // Kayıp olabilir
    } else if (from_type->base_type == TYPE_BOOL) {
        if (to_type->base_type == TYPE_INT) return true;
    } else if (from_type->base_type == TYPE_CHAR) {
        if (to_type->base_type == TYPE_INT) return true;
    }
    // ... diğer dönüşüm kuralları ...

    return false;
}

bool convert_int_to_float(int from_value, float* to_value) {
    *to_value = (float)from_value;
    return true;
}

bool convert_float_to_int(float from_value, int* to_value) {
    *to_value = (int)from_value; // Bilgi kaybı olabilir
    return true;
}

bool convert_int_to_bool(int from_value, bool* to_value) {
    *to_value = (from_value != 0);
    return true;
}

bool convert_bool_to_int(bool from_value, int* to_value) {
    *to_value = from_value ? 1 : 0;
    return true;
}

bool convert_char_to_int(char from_value, int* to_value) {
    *to_value = (int)from_value;
    return true;
}

bool convert_int_to_char(int from_value, char* to_value) {
    if (from_value >= CHAR_MIN && from_value <= CHAR_MAX) {
        *to_value = (char)from_value;
        return true;
    }
    return false; // Değer char aralığında değil
}

bool convert_any_to_string(const void* from_value, const Type* from_type, char** to_string) {
    if (from_type->base_type == TYPE_INT) {
        int value = *(const int*)from_value;
        *to_string = (char*)malloc(12); // Tahmini maksimum int uzunluğu
        sprintf(*to_string, "%d", value);
        return true;
    } else if (from_type->base_type == TYPE_FLOAT) {
        float value = *(const float*)from_value;
        *to_string = (char*)malloc(32); // Tahmini maksimum float uzunluğu
        sprintf(*to_string, "%f", value);
        return true;
    } else if (from_type->base_type == TYPE_BOOL) {
        bool value = *(const bool*)from_value;
        *to_string = strdup(value ? "true" : "false");
        return true;
    } else if (from_type->base_type == TYPE_CHAR) {
        char value = *(const char*)from_value;
        *to_string = (char*)malloc(2);
        sprintf(*to_string, "%c", value);
        (*to_string)[1] = '\0';
        return true;
    } else if (from_type->base_type == TYPE_STRING) {
        *to_string = strdup(*(const char**)from_value);
        return true;
    }
    return false; // Diğer tipler için dönüşüm henüz desteklenmiyor
}

 bool convert_string_to_any(const char* from_string, const Type* to_type, void* to_value) {
     if (to_type->base_type == TYPE_INT) {
         *(int*)to_value = atoi(from_string);
         return true;
     } else if (to_type->base_type == TYPE_FLOAT) {
         *(float*)to_value = atof(from_string);
         return true;
     } else if (to_type->base_type == TYPE_BOOL) {
         if (strcmp(from_string, "true") == 0) *(bool*)to_value = true;
         else if (strcmp(from_string, "false") == 0) *(bool*)to_value = false;
         else return false;
         return true;
     } else if (to_type->base_type == TYPE_CHAR) {
         if (strlen(from_string) == 1) *(char*)to_value = from_string[0];
         else return false;
         return true;
     }
     return false; // Diğer tiplere dönüşüm henüz desteklenmiyor
}