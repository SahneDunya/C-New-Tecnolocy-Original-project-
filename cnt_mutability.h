#ifndef CNT_MUTABILITY_H
#define CNT_MUTABILITY_H

#include <stdbool.h>
#include "cnt_variable.h" // Variable yapısı için

// Değiştirilebilirlik Durumları (isteğe bağlı)
typedef enum {
    MUTABLE,
    IMMUTABLE
} MutabilityStatus;

// Değişkenin değiştirilebilir olup olmadığını kontrol etme fonksiyonu
bool is_variable_mutable(const Variable* var);

// Bir değişkenin değiştirilebilirliğini ayarlama fonksiyonu
void set_variable_mutability(Variable* var, bool mutable);

#endif