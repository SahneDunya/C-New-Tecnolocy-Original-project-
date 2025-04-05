#ifndef CNT_METADATA_H
#define CNT_METADATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Meta Veri Türleri
typedef enum {
    METADATA_TYPE_INT,
    METADATA_TYPE_FLOAT,
    METADATA_TYPE_STRING,
    METADATA_TYPE_BOOL,
    // ... diğer meta veri türleri ...
} MetadataType;

// Meta Veri Yapısı
typedef struct Metadata {
    const char* key;
    MetadataType type;
    union {
        int64_t int_value;
        double float_value;
        char* string_value;
        bool bool_value;
        // ... diğer meta veri değerleri ...
    } value;
    struct Metadata* next;
} Metadata;

// Meta Veri Ekleme Fonksiyonu
Metadata* metadata_add(Metadata* metadata, const char* key, MetadataType type, ...);

// Meta Veri Alma Fonksiyonu
Metadata* metadata_get(Metadata* metadata, const char* key);

// Meta Veri Serbest Bırakma Fonksiyonu
void metadata_free(Metadata* metadata);

#endif