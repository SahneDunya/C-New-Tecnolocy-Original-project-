#ifndef CNT_OWNERSHIP_H
#define CNT_OWNERSHIP_H

#include <stdbool.h>
#include "cnt_variable.h" // Variable yapısı için

// Sahiplik Durumları
typedef enum {
    OWNED,      // Değerin tek sahibi var
    BORROWED,   // Değer ödünç alınmış
    UNOWNED     // Değerin sahibi yok (örneğin, statik veriler)
} OwnershipStatus;

// Ödünç Alma Türleri
typedef enum {
    BORROW_IMMUTABLE, // Değiştirilemez ödünç alma
    BORROW_MUTABLE    // Değiştirilebilir ödünç alma
} BorrowType;

// Ödünç Alma Bilgisi Yapısı
typedef struct Borrow {
    Variable* borrower; // Ödünç alan değişken
    BorrowType type;
    struct Borrow* next;
} Borrow;

// Değişken Yapısına Sahiplik Bilgisi Ekleme
typedef struct VariableOwnership {
    OwnershipStatus status;
    Variable* owner;      // Eğer ödünç alınmışsa, sahibi
    Borrow* borrows;      // Bu değişkene yapılan ödünç almaların listesi
} VariableOwnership;

 typedef struct Variable {
     char* name;
     Type* type;
     void* value;
     bool is_mutable;
     VariableOwnership* ownership; // Yeni eklenen alan
 };

// Sahiplik ve Ödünç Alma Yönetimi Fonksiyon Prototipleri

// Bir değişken için sahiplik bilgisi oluşturma
VariableOwnership* ownership_create(OwnershipStatus status, Variable* owner);

// Bir değişkene ödünç alma ekleme
bool ownership_add_borrow(Variable* owned_var, Variable* borrower, BorrowType type);

// Bir değişkenden ödünç alma kaldırma (kapsam dışına çıkıldığında vb.)
bool ownership_remove_borrow(Variable* owned_var, Variable* borrower);

// Bir değişkenin sahipliğini başka bir değişkene taşıma
bool ownership_transfer(Variable* from_var, Variable* to_var);

// Bir değişkenin ödünç alınıp alınamayacağını kontrol etme
bool can_borrow(const Variable* owned_var, BorrowType type);

// Bir değişkenin sahibi olup olmadığını kontrol etme
bool is_owner(const Variable* var);

// Bir değişkenin ödünç alınmış olup olmadığını kontrol etme
bool is_borrowed(const Variable* var);

// Bir değişkenin ödünç veren olup olmadığını kontrol etme
bool is_borrower(const Variable* var);

// Bir değişkenin kapsamı dışına çıkması durumunda yapılacak işlemler (örneğin, ödünç almaların kontrolü)
void ownership_variable_out_of_scope(Variable* var);

#endif