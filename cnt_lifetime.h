#ifndef CNT_LIFETIME_H
#define CNT_LIFETIME_H

#include <stdbool.h>
#include <stddef.h>

// Yaşam Süresi Yapısı
typedef struct Lifetime {
    void* owner; // Sahiplik sahibi
    size_t scope_id; // Kapsam kimliği
} Lifetime;

// Yaşam Süresi Oluşturma Fonksiyonu
Lifetime lifetime_create(void* owner, size_t scope_id);

// Yaşam Süresi Kontrol Fonksiyonu
bool lifetime_check(Lifetime* lifetime, size_t current_scope_id);

// Yaşam Süresi Serbest Bırakma Fonksiyonu
void lifetime_drop(Lifetime* lifetime);

#endif