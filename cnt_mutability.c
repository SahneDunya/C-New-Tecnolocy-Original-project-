#include "cnt_mutability.h"
#include <stdio.h>

bool is_variable_mutable(const Variable* var) {
    if (var == NULL) {
        fprintf(stderr, "Hata: Geçersiz değişken pointerı.\n");
        return false; // Veya uygun bir hata yönetimi mekanizması
    }
    return var->is_mutable;
}

void set_variable_mutability(Variable* var, bool mutable) {
    if (var == NULL) {
        fprintf(stderr, "Hata: Geçersiz değişken pointerı.\n");
        return; // Veya uygun bir hata yönetimi mekanizması
    }
    var->is_mutable = mutable;
}