#ifndef CNT_BORROW_H
#define CNT_BORROW_H

#include <stdbool.h>
#include <stddef.h>

// Ödünç Alma Yapısı
typedef struct Borrow {
    void* value; // Ödünç alınan değerin işaretçisi
    bool is_mutable; // Değiştirilebilir ödünç alma mı?
    size_t owner_scope_id; // Sahiplik sahibinin kapsam kimliği
} Borrow;

// Ödünç Alma Oluşturma Fonksiyonu
Borrow borrow_create(void* value, bool is_mutable, size_t owner_scope_id);

// Ödünç Alma Kontrol Fonksiyonu
bool borrow_check(Borrow* borrow, size_t current_scope_id);

// Ödünç Alma Serbest Bırakma Fonksiyonu
void borrow_drop(Borrow* borrow);

#endif