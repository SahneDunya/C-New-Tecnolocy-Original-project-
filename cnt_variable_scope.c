#include "cnt_variable_scope.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SymbolTable* scope_create(ScopeType type, SymbolTable* parent) {
    SymbolTable* new_scope = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (new_scope == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    new_scope->type = type;
    new_scope->parent = parent;
    new_scope->symbols = NULL; // Sembol tablosu (değişkenler, fonksiyonlar vb.)
    new_scope->symbol_count = 0;
    return new_scope;
}

bool scope_add_variable(SymbolTable* scope, Variable* var) {
    if (scope == NULL || var == NULL) {
        return false;
    }
    // Aynı isimde bir değişkenin aynı kapsamda olup olmadığını kontrol etme (shadowing'e izin verilebilir)
    // ...

    // Sembol tablosuna değişkeni ekleme (basit bir örnek)
    if (scope->symbols == NULL) {
        scope->symbols = (Symbol*)malloc(sizeof(Symbol));
    } else {
        scope->symbols = (Symbol*)realloc(scope->symbols, (scope->symbol_count + 1) * sizeof(Symbol));
    }
    if (scope->symbols == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    scope->symbols[scope->symbol_count].name = strdup(var->name);
    scope->symbols[scope->symbol_count].type = SYMBOL_VARIABLE;
    scope->symbols[scope->symbol_count].data = var;
    scope->symbol_count++;
    return true;
}

Variable* scope_find_variable(SymbolTable* scope, const char* name) {
    SymbolTable* current_scope = scope;
    while (current_scope != NULL) {
        for (size_t i = 0; i < current_scope->symbol_count; ++i) {
            if (current_scope->symbols[i].type == SYMBOL_VARIABLE &&
                strcmp(current_scope->symbols[i].name, name) == 0) {
                return (Variable*)current_scope->symbols[i].data;
            }
        }
        current_scope = current_scope->parent; // Üst kapsamda ara
    }
    return NULL; // Değişken bulunamadı
}

void scope_print(const SymbolTable* scope) {
    if (scope == NULL) {
        printf("Kapsam: NULL\n");
        return;
    }
    printf("Kapsam Türü: %d, Sembol Sayısı: %zu\n", scope->type, scope->symbol_count);
    for (size_t i = 0; i < scope->symbol_count; ++i) {
        if (scope->symbols[i].type == SYMBOL_VARIABLE) {
            Variable* var = (Variable*)scope->symbols[i].data;
            printf("  Değişken: Ad=%s, Tip=", var->name);
            if (var->type) {
                printf("%s", var->type->name);
            } else {
                printf("Bilinmiyor");
            }
            printf(", Değiştirilebilir=%s\n", var->is_mutable ? "Evet" : "Hayır");
        } else {
            printf("  Diğer Sembol: Ad=%s, Tür=%d\n", scope->symbols[i].name, scope->symbols[i].type);
        }
    }
    if (scope->parent != NULL) {
        printf("Üst Kapsam:\n");
        scope_print(scope->parent);
    } else {
        printf("Üst Kapsam: Yok\n");
    }
}