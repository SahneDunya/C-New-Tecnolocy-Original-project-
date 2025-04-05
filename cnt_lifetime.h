#ifndef CNT_LIFETIME_H
#define CNT_LIFETIME_H

#include <stdbool.h>
#include "cnt_variable.h" // Variable yapısı için

// Ömür Kategorileri
typedef enum {
    LIFETIME_STATIC,   // Programın başlangıcından sonuna kadar yaşar
    LIFETIME_AUTOMATIC, // Tanımlandığı blok boyunca yaşar
    LIFETIME_TEMPORARY // Bir ifade veya işlem süresince yaşar
    // ... diğer ömür kategorileri gerekirse eklenebilir
} LifetimeCategory;

// Ömür Bilgisi Yapısı
typedef struct LifetimeInfo {
    LifetimeCategory category;
    // Otomatik ömür için, tanımlandığı kapsamı tutabiliriz (isteğe bağlı)
    // SymbolTable* scope;
    // ... ömür ile ilgili ek bilgiler gerekirse eklenebilir
} LifetimeInfo;

 typedef struct Variable {
     char* name;
     Type* type;
     void* value;
     bool is_mutable;
     VariableOwnership* ownership;
     LifetimeInfo* lifetime; // Yeni eklenen alan
 };

// Ömür Yönetimi Fonksiyon Prototipleri

// Bir değişken için ömür bilgisi oluşturma
LifetimeInfo* lifetime_create(LifetimeCategory category);

// Bir değişkene ömür bilgisi atama
void lifetime_assign(Variable* var, LifetimeCategory category);

// İki değişkenin ömürlerinin uyumlu olup olmadığını kontrol etme
// (Örneğin, ödünç alanın ömrü, ödünç verenin ömründen kısa veya eşit olmalı)
bool lifetimes_are_compatible(const Variable* borrower, const Variable* owner);

// Bir değişkenin kapsamının sonuna ulaşıp ulaşmadığını kontrol etme (basit bir yaklaşım)
bool is_lifetime_ended(const Variable* var);

#endif