#include "cnt_borrow.h"
#include <stdlib.h>

// Ödünç Alma Oluşturma Fonksiyonu
Borrow borrow_create(void* value, bool is_mutable, size_t owner_scope_id) {
    Borrow borrow = {value, is_mutable, owner_scope_id};
    return borrow;
}

// Ödünç Alma Kontrol Fonksiyonu
bool borrow_check(Borrow* borrow, size_t current_scope_id) {
    // Ödünç alınan referansın kapsamı, sahibinin kapsamından daha uzun olmamalı
    return borrow->owner_scope_id <= current_scope_id;
}

// Ödünç Alma Serbest Bırakma Fonksiyonu
void borrow_drop(Borrow* borrow) {
    // Ödünç alma bittiğinde, ödünç alınan referansın sahipliği serbest bırakılır
    // (Gerçek uygulamada, sahiplik serbest bırakma mantığı burada uygulanmalıdır)
}