#ifndef CNT_VARIABLE_SCOPE_H
#define CNT_VARIABLE_SCOPE_H

#include <stdbool.h>
#include "cnt_symboltable.h" // SymbolTable ve ScopeType için
#include "cnt_variable.h"    // Variable yapısı için

// Yeni bir kapsam oluşturma fonksiyonu
SymbolTable* scope_create(ScopeType type, SymbolTable* parent);

// Geçerli kapsama bir değişken ekleme fonksiyonu
bool scope_add_variable(SymbolTable* scope, Variable* var);

// Geçerli kapsamda veya üst kapsamlarında bir değişkeni adına göre bulma fonksiyonu
Variable* scope_find_variable(SymbolTable* scope, const char* name);

// Geçerli kapsamı yazdırma (debug için)
void scope_print(const SymbolTable* scope);

#endif