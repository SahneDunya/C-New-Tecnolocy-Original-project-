#ifndef CNT_VARIABLE_H
#define CNT_VARIABLE_H

#include <stdbool.h>
#include "cnt_type.h" // Type yapısı için

// Değişken Yapısı
typedef struct Variable {
    char* name;
    Type* type;
    void* value; // Değişkenin değerini tutan genel amaçlı pointer
    bool is_mutable; // Değişkenin değiştirilebilir olup olmadığı
    // Modern özellikler için eklenebilir:
    bool is_owned;
    struct Variable* owner; // Eğer ödünç alınmışsa
    int lifetime_counter;
} Variable;

// Yeni bir değişken oluşturma fonksiyonu
Variable* variable_create(const char* name, Type* type, bool mutable);

// Bir değişkenin değerini ayarlama fonksiyonu
bool variable_set_value(Variable* var, const void* new_value);

// Bir değişkenin değerini alma fonksiyonu
const void* variable_get_value(const Variable* var);

// Bir değişkeni serbest bırakma fonksiyonu (değerin kendisi değil, değişken yapısı)
void variable_free(Variable* var);

// Değişkenin tipini yazdırma (debug için)
void variable_print(const Variable* var);

#endif