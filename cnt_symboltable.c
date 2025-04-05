#include "cnt_symboltable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SymbolTable* symboltable_create(SymbolTable* parent) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (table == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    table->head = NULL;
    table->parent = parent;
    return table;
}

void symboltable_add(SymbolTable* table, const char* name, Type* type, ScopeType scope) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if (new_symbol == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    new_symbol->name = strdup(name);
    new_symbol->type = type;
    new_symbol->scope = scope;
    new_symbol->next = table->head;
    table->head = new_symbol;
}

Symbol* symboltable_find(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    if (table->parent != NULL) {
        return symboltable_find(table->parent, name);
    }
    return NULL;
}

Symbol* symboltable_find_local(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void symboltable_free(SymbolTable* table) {
    Symbol* current = table->head;
    while (current != NULL) {
        Symbol* next = current->next;
        free(current->name);
        // current->type serbest bırakılmamalı, çünkü tip tanımları genellikle tek bir yerde tutulur
        free(current);
        current = next;
    }
    free(table);
}