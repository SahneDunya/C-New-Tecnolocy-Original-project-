#include "cnt_ownership.h"
#include <stdlib.h>

// Sahiplik Oluşturma Fonksiyonu
Ownership ownership_create(void* value) {
    Ownership ownership = {value, true};
    return ownership;
}

// Sahiplik Devretme Fonksiyonu
Ownership ownership_move(Ownership* owner) {
    if (!owner->is_owned) {
        // Hata: Sahiplik zaten devredilmiş
        return ownership_create(NULL);
    }
    Ownership new_ownership = {owner->value, true};
    owner->is_owned = false;
    return new_ownership;
}

// Sahiplik Ödünç Alma Fonksiyonu
void* ownership_borrow(Ownership* owner) {
    if (!owner->is_owned) {
        // Hata: Sahiplik devredilmiş veya serbest bırakılmış
        return NULL;
    }
    return owner->value;
}

// Sahiplik Değiştirilebilir Ödünç Alma Fonksiyonu
void* ownership_borrow_mut(Ownership* owner) {
    if (!owner->is_owned) {
        // Hata: Sahiplik devredilmiş veya serbest bırakılmış
        return NULL;
    }
    return owner->value;
}

// Sahiplik Serbest Bırakma Fonksiyonu
void ownership_drop(Ownership* owner) {
    if (owner->is_owned) {
        free(owner->value);
        owner->is_owned = false;
    }
}