#include "cnt_ownership.h"
#include <stdlib.h>
#include <stdio.h>

VariableOwnership* ownership_create(OwnershipStatus status, Variable* owner) {
    VariableOwnership* ownership = (VariableOwnership*)malloc(sizeof(VariableOwnership));
    if (!ownership) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    ownership->status = status;
    ownership->owner = owner;
    ownership->borrows = NULL;
    return ownership;
}

bool ownership_add_borrow(Variable* owned_var, Variable* borrower, BorrowType type) {
    if (owned_var == NULL || borrower == NULL) {
        fprintf(stderr, "Hata: Geçersiz değişken pointerı.\n");
        return false;
    }

    // Ödünç alma kurallarını kontrol etme (basit bir örnek)
    Borrow* current_borrow = owned_var->ownership->borrows;
    while (current_borrow != NULL) {
        if (current_borrow->borrower == borrower) {
            fprintf(stderr, "Hata: Bu değişken zaten ödünç alınmış.\n");
            return false;
        }
        if (type == BORROW_MUTABLE && current_borrow->type == BORROW_IMMUTABLE) {
            fprintf(stderr, "Hata: Değiştirilebilir ödünç alma varken değiştirilemez ödünç alma yapılamaz.\n");
            return false;
        }
        if (type == BORROW_IMMUTABLE && current_borrow->type == BORROW_MUTABLE) {
            fprintf(stderr, "Hata: Değiştirilemez ödünç alma varken değiştirilebilir ödünç alma mevcut.\n");
            return false;
        }
        current_borrow = current_borrow->next;
    }

    Borrow* new_borrow = (Borrow*)malloc(sizeof(Borrow));
    if (!new_borrow) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    new_borrow->borrower = borrower;
    new_borrow->type = type;
    new_borrow->next = owned_var->ownership->borrows;
    owned_var->ownership->borrows = new_borrow;
    return true;
}

bool ownership_remove_borrow(Variable* owned_var, Variable* borrower) {
    if (owned_var == NULL || borrower == NULL || owned_var->ownership == NULL) {
        return false;
    }
    Borrow *current = owned_var->ownership->borrows;
    Borrow *prev = NULL;
    while (current != NULL) {
        if (current->borrower == borrower) {
            if (prev == NULL) {
                owned_var->ownership->borrows = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool ownership_transfer(Variable* from_var, Variable* to_var) {
    if (from_var == NULL || to_var == NULL || from_var->ownership == NULL || to_var->ownership == NULL) {
        return false;
    }
    if (from_var->ownership->status != OWNED) {
        fprintf(stderr, "Hata: Sahipliği aktarılacak değişkenin sahibi olması gerekiyor.\n");
        return false;
    }
    if (to_var->ownership->status != UNOWNED) {
        fprintf(stderr, "Hata: Sahipliğin aktarılacağı değişkenin sahibi olmaması gerekiyor.\n");
        return false;
    }
    to_var->ownership->status = OWNED;
    from_var->ownership->status = UNOWNED;
    // Gerekirse diğer bilgileri de aktarabilirsiniz
    return true;
}

bool can_borrow(const Variable* owned_var, BorrowType type) {
    if (owned_var == NULL || owned_var->ownership == NULL) {
        return false;
    }
    Borrow* current_borrow = owned_var->ownership->borrows;
    while (current_borrow != NULL) {
        if (type == BORROW_MUTABLE && current_borrow->type == BORROW_IMMUTABLE) {
            return false;
        }
        if (type == BORROW_IMMUTABLE && current_borrow->type == BORROW_MUTABLE) {
            return false;
        }
        current_borrow = current_borrow->next;
    }
    return true;
}

bool is_owner(const Variable* var) {
    return var != NULL && var->ownership != NULL && var->ownership->status == OWNED;
}

bool is_borrowed(const Variable* var) {
    return var != NULL && var->ownership != NULL && var->ownership->status == BORROWED;
}

bool is_borrower(const Variable* var) {
    if (var == NULL || var->ownership == NULL) {
        return false;
    }
    return var->ownership->borrows != NULL;
}

void ownership_variable_out_of_scope(Variable* var) {
    if (var == NULL || var->ownership == NULL) {
        return;
    }
    // Ödünç almaların geçerliliğini kontrol etme (gerçek bir implementasyon daha karmaşık olabilir)
    if (var->ownership->borrows != NULL) {
        fprintf(stderr, "Uyarı: Kapsam dışına çıkan değişken hala ödünç alınmış durumda.\n");
        // Burada ödünç alan değişkenlere hata verilebilir veya başka işlemler yapılabilir.
    }
    // Eğer sahibi ise, değeri serbest bırakma mantığı buraya eklenebilir (basit C'de malloc/free varsayılır).
    if (var->ownership->status == OWNED) {
        // free(var->value); // Değerin ne zaman serbest bırakılacağını belirlemek karmaşıktır.
    }
    // Sahiplik bilgisini serbest bırakma
    Borrow* current = var->ownership->borrows;
    while (current != NULL) {
        Borrow* next = current->next;
        free(current);
        current = next;
    }
    free(var->ownership);
    var->ownership = NULL;
}