#include "cnt_scope.h"
#include <stdlib.h>

// Kapsam Oluşturma Fonksiyonu
Scope* scope_create(size_t id, Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    if (scope == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    scope->id = id;
    scope->parent = parent;
    scope->variables = NULL;
    scope->variable_count = 0;
    return scope;
}

// Değişken Ekleme Fonksiyonu
bool scope_add_variable(Scope* scope, void* variable) {
    scope->variables = (void**)realloc(scope->variables, sizeof(void*) * (scope->variable_count + 1));
    if (scope->variables == NULL) {
        return false; // Bellek ayırma hatası
    }
    scope->variables[scope->variable_count] = variable;
    scope->variable_count++;
    return true;
}

// Değişken Alma Fonksiyonu
void* scope_get_variable(Scope* scope, size_t index) {
    if (index < scope->variable_count) {
        return scope->variables[index];
    }
    return NULL; // Geçersiz indeks
}

// Kapsam Serbest Bırakma Fonksiyonu
void scope_free(Scope* scope) {
    free(scope->variables);
    free(scope);
}