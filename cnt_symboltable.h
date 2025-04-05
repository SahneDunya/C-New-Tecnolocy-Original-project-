#ifndef CNT_SYMBOLTABLE_H
#define CNT_SYMBOLTABLE_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_type.h" // Tipleri temsil etmek için

// Kapsam Türleri
typedef enum {
    SCOPE_GLOBAL,
    SCOPE_MODULE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK
    // ... diğer kapsam türleri ...
} ScopeType;

// Sembol Tablosu Girişi Yapısı
typedef struct Symbol {
    char* name;
    Type* type;
    ScopeType scope;
    bool is_mutable;
    bool is_owned;
    struct Symbol* owner;
    bool is_exported;
    // ...
    struct Symbol* next;
} Symbol;

// Sembol Tablosu Yapısı
typedef struct SymbolTable {
    struct Symbol* head;
    struct SymbolTable* parent; // İç içe kapsamlar için
} SymbolTable;

// Sembol Tablosu Oluşturma Fonksiyonu
SymbolTable* symboltable_create(SymbolTable* parent);

// Sembol Ekleme Fonksiyonu
void symboltable_add(SymbolTable* table, const char* name, Type* type, ScopeType scope);

// Sembol Arama Fonksiyonu (geçerli kapsam ve üst kapsamlar)
Symbol* symboltable_find(SymbolTable* table, const char* name);

// Sadece geçerli kapsamda sembol arama Fonksiyonu
Symbol* symboltable_find_local(SymbolTable* table, const char* name);

// Sembol Tablosunu Serbest Bırakma Fonksiyonu
void symboltable_free(SymbolTable* table);

#endif