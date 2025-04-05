#ifndef CNT_SCOPE_H
#define CNT_SCOPE_H

#include <stdbool.h>
#include <stddef.h>

// Kapsam Yapısı
typedef struct Scope {
    size_t id; // Kapsam kimliği
    struct Scope* parent; // Üst kapsam
    void** variables; // Kapsamdaki değişkenler
    size_t variable_count;
} Scope;

// Kapsam Oluşturma Fonksiyonu
Scope* scope_create(size_t id, Scope* parent);

// Değişken Ekleme Fonksiyonu
bool scope_add_variable(Scope* scope, void* variable);

// Değişken Alma Fonksiyonu
void* scope_get_variable(Scope* scope, size_t index);

// Kapsam Serbest Bırakma Fonksiyonu
void scope_free(Scope* scope);

#endif