#include "cnt_lifetime.h"
#include <stdlib.h>

// Yaşam Süresi Oluşturma Fonksiyonu
Lifetime lifetime_create(void* owner, size_t scope_id) {
    Lifetime lifetime = {owner, scope_id};
    return lifetime;
}

// Yaşam Süresi Kontrol Fonksiyonu
bool lifetime_check(Lifetime* lifetime, size_t current_scope_id) {
    // Ödünç alınan referansın kapsamı, sahibinin kapsamından daha uzun olmamalı
    return lifetime->scope_id <= current_scope_id;
}

// Yaşam Süresi Serbest Bırakma Fonksiyonu
void lifetime_drop(Lifetime* lifetime) {
    // Yaşam süresi bittiğinde, ödünç alınan referansın sahipliği serbest bırakılır
    // (Gerçek uygulamada, sahiplik serbest bırakma mantığı burada uygulanmalıdır)
}