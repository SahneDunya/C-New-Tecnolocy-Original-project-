#ifndef CNT_MODULE_H
#define CNT_MODULE_H

#include <stdbool.h>
#include <stddef.h>

// Modül Yapısı
typedef struct Module {
    char* name;
    void* handle; // Modülün yüklenme tutamacı
    struct Module** dependencies; // Bağımlılıklar
    size_t dependency_count;
    bool (*init)(void);
    void (*cleanup)(void);
} Module;

// Modül Oluşturma Fonksiyonu
Module* module_create(const char* name, bool (*init)(void), void (*cleanup)(void));

// Modül Bağımlılığı Ekleme Fonksiyonu
bool module_add_dependency(Module* module, Module* dependency);

// Modül Yükleme Fonksiyonu
bool module_load(Module* module);

// Modül Kaldırma Fonksiyonu
void module_unload(Module* module);

// Modül Serbest Bırakma Fonksiyonu
void module_free(Module* module);

#endif