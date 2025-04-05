#include "cnt_metadata.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Meta Veri Ekleme Fonksiyonu
Metadata* metadata_add(Metadata* metadata, const char* key, MetadataType type, ...) {
    Metadata* new_metadata = (Metadata*)malloc(sizeof(Metadata));
    if (new_metadata == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    new_metadata->key = strdup(key);
    new_metadata->type = type;
    new_metadata->next = NULL;

    va_list args;
    va_start(args, type);

    switch (type) {
        case METADATA_TYPE_INT:
            new_metadata->value.int_value = va_arg(args, int64_t);
            break;
        case METADATA_TYPE_FLOAT:
            new_metadata->value.float_value = va_arg(args, double);
            break;
        case METADATA_TYPE_STRING:
            new_metadata->value.string_value = strdup(va_arg(args, char*));
            break;
        case METADATA_TYPE_BOOL:
            new_metadata->value.bool_value = va_arg(args, int); // bool, int olarak geçirilir
            break;
        // ... diğer meta veri türleri ...
    }

    va_end(args);

    if (metadata == NULL) {
        return new_metadata;
    }

    Metadata* current = metadata;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_metadata;

    return metadata;
}

// Meta Veri Alma Fonksiyonu
Metadata* metadata_get(Metadata* metadata, const char* key) {
    Metadata* current = metadata;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; // Meta veri bulunamadı
}

// Meta Veri Serbest Bırakma Fonksiyonu
void metadata_free(Metadata* metadata) {
    Metadata* current = metadata;
    while (current != NULL) {
        Metadata* next = current->next;
        free((void*)current->key);
        if (current->type == METADATA_TYPE_STRING) {
            free(current->value.string_value);
        }
        free(current);
        current = next;
    }
}