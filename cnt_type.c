#include "cnt_type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Type* type_create(BaseType base_type, const char* name) {
    Type* type = (Type*)malloc(sizeof(Type));
    if (type == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    type->base_type = base_type;
    type->name = strdup(name);
    // Detaylar için varsayılan değerler atanabilir
    return type;
}

Type* type_create_pointer(Type* points_to) {
    Type* type = type_create(TYPE_POINTER, NULL);
    type->details.pointer_to = points_to;
    // İsim oluşturulabilir (örneğin, "*int")
    size_t name_len = strlen(points_to->name) + 2;
    type->name = (char*)malloc(name_len);
    snprintf(type->name, name_len, "*%s", points_to->name);
    return type;
}

Type* type_create_array(Type* element_type, size_t size) {
    Type* type = type_create(TYPE_ARRAY, NULL);
    type->details.array.element_type = element_type;
    type->details.array.size = size;
    // İsim oluşturulabilir (örneğin, "int[10]")
    size_t name_len = strlen(element_type->name) + 10; // Tahmini uzunluk
    type->name = (char*)malloc(name_len);
    snprintf(type->name, name_len, "%s[%zu]", element_type->name, size);
    return type;
}

bool types_are_compatible(const Type* type1, const Type* type2) {
    if (type1 == NULL || type2 == NULL) {
        return false;
    }
    if (type1->base_type != type2->base_type) {
        return false;
    }
    // Ek kontroller (örneğin, pointer'ın işaret ettiği tip, array'in eleman tipi)
    if (type1->base_type == TYPE_POINTER && type2->base_type == TYPE_POINTER) {
        return types_are_compatible(type1->details.pointer_to, type2->details.pointer_to);
    }
    if (type1->base_type == TYPE_ARRAY && type2->base_type == TYPE_ARRAY) {
        return type1->details.array.size == type2->details.array.size &&
               types_are_compatible(type1->details.array.element_type, type2->details.array.element_type);
    }
    // İsim bazlı karşılaştırma (temel tipler ve kullanıcı tanımlı tipler için)
    if (type1->name != NULL && type2->name != NULL) {
        return strcmp(type1->name, type2->name) == 0;
    }
    // İsim yoksa base type'a göre karar verilebilir (dikkatli olunmalı)
    return type1->base_type == type2->base_type;
}

void type_print(const Type* type) {
    if (type == NULL) {
        printf("null");
        return;
    }
    printf("%s", type->name);
}