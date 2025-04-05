#ifndef CNT_TYPE_H
#define CNT_TYPE_H

#include <stdbool.h>
#include <stddef.h>

// Temel Veri Tipleri (enum olarak tanımlanabilir)
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
    TYPE_CUSTOM // Kullanıcı tanımlı tipler için
    // ... diğer tipler (pointer, array vb.) ...
} BaseType;

// Tip Yapısı
typedef struct Type {
    BaseType base_type;
    char* name; // Tipin adı (örneğin, "int", "MyStruct")
    struct Type* element_type; // Array veya pointer için
    size_t size; // Tipin boyutu
    // ...
} Type;

// Tip Oluşturma Fonksiyonu (cnt_semantic_analyzer.c içinde tanımlı, burada prototipi olabilir)
Type* type_create(const char* name);

// Tiplerin Uyumlu Olup Olmadığını Kontrol Etme Fonksiyonu (cnt_semantic_analyzer.c içinde tanımlı, burada prototipi olabilir)
bool types_are_compatible(Type* type1, Type* type2);

#endif