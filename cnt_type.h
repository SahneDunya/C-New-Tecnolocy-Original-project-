#ifndef CNT_TYPE_H
#define CNT_TYPE_H

#include <stdbool.h>
#include <stddef.h>

// Temel Veri Tipleri
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_MODULE,
    TYPE_INTERFACE,
    TYPE_FUNCTION,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_CUSTOM // Kullanıcı tanımlı tipler için
    // Modern özellikler için eklenebilir:
    TYPE_OWNED,
    TYPE_BORROWED,
    // ...
} BaseType;

// Tip Yapısı
typedef struct Type {
    BaseType base_type;
    char* name; // Tipin adı (örneğin, "int", "MyStruct", "*int")
    // Ek bilgiler için union kullanılabilir
    union {
        struct Type* pointer_to; // TYPE_POINTER için işaret ettiği tip
        struct {
            struct Type* element_type; // TYPE_ARRAY için eleman tipi
            size_t size; // TYPE_ARRAY için boyutu (derleme zamanında biliniyorsa)
        } array;
        struct { // TYPE_STRUCT ve TYPE_ENUM için alanlar eklenebilir
        //     // ...
         } composite;
    } details;
    bool is_mutable;
    bool is_owned;
    bool is_borrowed;
} Type;

// Tip Oluşturma Fonksiyonu
Type* type_create(BaseType base_type, const char* name);
Type* type_create_pointer(Type* points_to);
Type* type_create_array(Type* element_type, size_t size);

// Tiplerin Uyumlu Olup Olmadığını Kontrol Etme Fonksiyonu
bool types_are_compatible(const Type* type1, const Type* type2);

// Tip Bilgisini Yazdırma Fonksiyonu (debug için)
void type_print(const Type* type);

#endif