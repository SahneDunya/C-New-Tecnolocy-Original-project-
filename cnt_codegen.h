#ifndef CNT_CODEGEN_H
#define CNT_CODEGEN_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_metadata.h" // Meta verileri kullanmak için

// Kod Üretimi Yapısı
typedef struct CodeGenerator {
    Metadata* metadata; // Kod üretimi için kullanılacak meta veriler
} CodeGenerator;

// Kod Üretici Oluşturma Fonksiyonu
CodeGenerator* codegen_create(Metadata* metadata);

// Kod Üretme Fonksiyonu
char* codegen_generate(CodeGenerator* generator, const char* template);

// Kod Üreticiyi Serbest Bırakma Fonksiyonu
void codegen_free(CodeGenerator* generator);

#endif