#ifndef CNT_COMPOSITE_TYPES_H
#define CNT_COMPOSITE_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_type.h" // Type yapısı için

// Dizi Tipi
typedef struct ArrayType {
    Type* element_type;
    size_t size; // Derleme zamanında bilinen boyut
    // Modern özellikler için eklenebilir:
    // bool is_dynamic; // Dinamik dizi olup olmadığı
} ArrayType;

// Yapı (Struct) Tipi
typedef struct StructMember {
    char* name;
    Type* type;
    struct StructMember* next;
} StructMember;

typedef struct StructType {
    char* name; // Yapı adı
    StructMember* members;
    bool has_methods;
} StructType;

// Birlik (Union) Tipi
typedef struct UnionMember {
    char* name;
    Type* type;
    struct UnionMember* next;
} UnionMember;

typedef struct UnionType {
    char* name; // Birlik adı
    UnionMember* members;
} UnionType;

// Enum Tipi
typedef struct EnumMember {
    char* name;
    int value; // İsteğe bağlı değer
    struct EnumMember* next;
} EnumMember;

typedef struct EnumType {
    char* name; // Enum adı
    EnumMember* members;
} EnumType;

// Birleşik Tip Oluşturma Fonksiyonları (prototipler)
Type* composite_type_create_array(Type* element_type, size_t size);
Type* composite_type_create_struct(const char* name, StructMember* members);
Type* composite_type_create_union(const char* name, UnionMember* members);
Type* composite_type_create_enum(const char* name, EnumMember* members);

// Yardımcı Fonksiyonlar (prototipler)
StructMember* struct_member_create(const char* name, Type* type, StructMember* next);
UnionMember* union_member_create(const char* name, Type* type, UnionMember* next);
EnumMember* enum_member_create(const char* name, int value, EnumMember* next);

#endif