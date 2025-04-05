#include "cnt_codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Kod Üretici Oluşturma Fonksiyonu
CodeGenerator* codegen_create(Metadata* metadata) {
    CodeGenerator* generator = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    if (generator == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    generator->metadata = metadata;
    return generator;
}

// Kod Üretme Fonksiyonu
char* codegen_generate(CodeGenerator* generator, const char* template) {
    // Şablonu ayrıştır ve meta verileri kullanarak kodu üret
    // ... şablon ayrıştırma ve kod üretme mantığı ...

    // Basit bir örnek: meta verideki "name" anahtarını şablona yerleştir
    Metadata* name_metadata = metadata_get(generator->metadata, "name");
    if (name_metadata == NULL || name_metadata->type != METADATA_TYPE_STRING) {
        return NULL; // "name" meta verisi bulunamadı veya yanlış türde
    }

    // Üretilen kod için bellek ayırma
    char* generated_code = (char*)malloc(strlen(template) + strlen(name_metadata->value.string_value) + 1);
    if (generated_code == NULL) {
        return NULL; // Bellek ayırma hatası
    }

    // Şablonu ve meta veriyi kullanarak kodu üret
    sprintf(generated_code, template, name_metadata->value.string_value);

    return generated_code;
}

// Kod Üreticiyi Serbest Bırakma Fonksiyonu
void codegen_free(CodeGenerator* generator) {
    free(generator);
}