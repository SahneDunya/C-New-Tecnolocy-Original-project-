#include "cnt_composite_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Birleşik Tip Oluşturma Fonksiyonları (implementasyonlar)
Type* composite_type_create_array(Type* element_type, size_t size) {
    Type* type = type_create(TYPE_ARRAY, NULL);
    type->details.array.element_type = element_type;
    type->details.array.size = size;
    // İsim oluşturulabilir (örneğin, "int[10]")
    size_t name_len = strlen(element_type->name) + 10; // Tahmini uzunluk
    type->name = (char*)malloc(name_len);
    snprintf(type->name, name_len, "%s[%zu]", element_type->name, size);
    return type;
}

Type* composite_type_create_struct(const char* name, StructMember* members) {
    Type* type = type_create(TYPE_STRUCT, strdup(name));
    StructType* struct_type = (StructType*)malloc(sizeof(StructType));
    if (!struct_type) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    struct_type->name = strdup(name);
    struct_type->members = members;
    type->details.composite = struct_type; // Union'da composite olarak saklıyoruz
    return type;
}

Type* composite_type_create_union(const char* name, UnionMember* members) {
    Type* type = type_create(TYPE_UNION, strdup(name));
    UnionType* union_type = (UnionType*)malloc(sizeof(UnionType));
    if (!union_type) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    union_type->name = strdup(name);
    union_type->members = members;
    type->details.composite = union_type; // Union'da composite olarak saklıyoruz
    return type;
}

Type* composite_type_create_enum(const char* name, EnumMember* members) {
    Type* type = type_create(TYPE_ENUM, strdup(name));
    EnumType* enum_type = (EnumType*)malloc(sizeof(EnumType));
    if (!enum_type) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    enum_type->name = strdup(name);
    enum_type->members = members;
    type->details.composite = enum_type; // Union'da composite olarak saklıyoruz
    return type;
}

// Yardımcı Fonksiyonlar (implementasyonlar)
StructMember* struct_member_create(const char* name, Type* type, StructMember* next) {
    StructMember* member = (StructMember*)malloc(sizeof(StructMember));
    if (!member) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    member->name = strdup(name);
    member->type = type;
    member->next = next;
    return member;
}

UnionMember* union_member_create(const char* name, Type* type, UnionMember* next) {
    UnionMember* member = (UnionMember*)malloc(sizeof(UnionMember));
    if (!member) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    member->name = strdup(name);
    member->type = type;
    member->next = next;
    return member;
}

EnumMember* enum_member_create(const char* name, int value, EnumMember* next) {
    EnumMember* member = (EnumMember*)malloc(sizeof(EnumMember));
    if (!member) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    member->name = strdup(name);
    member->value = value;
    member->next = next;
    return member;
}