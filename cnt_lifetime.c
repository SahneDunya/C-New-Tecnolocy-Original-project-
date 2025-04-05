#include "cnt_lifetime.h"
#include <stdlib.h>
#include <stdio.h>

LifetimeInfo* lifetime_create(LifetimeCategory category) {
    LifetimeInfo* lifetime = (LifetimeInfo*)malloc(sizeof(LifetimeInfo));
    if (!lifetime) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    lifetime->category = category;
    return lifetime;
}

void lifetime_assign(Variable* var, LifetimeCategory category) {
    if (var == NULL) {
        fprintf(stderr, "Hata: Geçersiz değişken pointerı.\n");
        return;
    }
    if (var->lifetime == NULL) {
        var->lifetime = lifetime_create(category);
    } else {
        var->lifetime->category = category;
    }
}

bool lifetimes_are_compatible(const Variable* borrower, const Variable* owner) {
    if (borrower == NULL || owner == NULL || borrower->lifetime == NULL || owner->lifetime == NULL) {
        return true; // Geçersiz durum, daha detaylı hata yönetimi gerekebilir
    }

    // Ödünç alanın ömrü, ödünç verenin ömründen daha uzun olamaz.
    if (borrower->lifetime->category > owner->lifetime->category) {
        fprintf(stderr, "Hata: Ödünç alanın ömrü, ödünç verenin ömründen daha uzun olamaz.\n");
        return false;
    }

    return true;
}

bool is_lifetime_ended(const Variable* var) {
    if (var == NULL || var->lifetime == NULL) {
        return false; // Geçersiz durum
    }
    // Bu fonksiyonun gerçek implementasyonu, derleyicinin kapsam yönetimi ve kontrol akışı analizine bağlı olacaktır.
    // Basit bir örnek olarak, otomatik ömürlü değişkenlerin kapsam dışına çıkıp çıkmadığını kontrol edebiliriz.
    // Ancak bu, semantik analiz aşamasında daha doğru bir şekilde belirlenir.
    return false; // Şimdilik her zaman false döndürüyoruz
}