#ifndef CNT_OWNERSHIP_H
#define CNT_OWNERSHIP_H

#include <stdbool.h>
#include <stddef.h>

// Sahiplik Yapısı
typedef struct Ownership {
    void* value;
    bool is_owned;
} Ownership;

// Sahiplik Oluşturma Fonksiyonu
Ownership ownership_create(void* value);

// Sahiplik Devretme Fonksiyonu
Ownership ownership_move(Ownership* owner);

// Sahiplik Ödünç Alma Fonksiyonu
void* ownership_borrow(Ownership* owner);

// Sahiplik Değiştirilebilir Ödünç Alma Fonksiyonu
void* ownership_borrow_mut(Ownership* owner);

// Sahiplik Serbest Bırakma Fonksiyonu
void ownership_drop(Ownership* owner);

#endif