#include "cnt_module.h"
#include <stdlib.h>
#include <string.h>

// Modül Oluşturma Fonksiyonu
Module* module_create(const char* name, bool (*init)(void), void (*cleanup)(void)) {
    Module* module = (Module*)malloc(sizeof(Module));
    if (module == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    module->name = strdup(name);
    module->handle = NULL;
    module->dependencies = NULL;
    module->dependency_count = 0;
    module->init = init;
    module->cleanup = cleanup;
    return module;
}

// Modül Bağımlılığı Ekleme Fonksiyonu
bool module_add_dependency(Module* module, Module* dependency) {
    module->dependencies = (Module**)realloc(module->dependencies, sizeof(Module*) * (module->dependency_count + 1));
    if (module->dependencies == NULL) {
        return false; // Bellek ayırma hatası
    }
    module->dependencies[module->dependency_count] = dependency;
    module->dependency_count++;
    return true;
}

// Modül Yükleme Fonksiyonu
bool module_load(Module* module) {
    // Modülün bağımlılıklarını yükleme
    for (size_t i = 0; i < module->dependency_count; i++) {
        if (!module_load(module->dependencies[i])) {
            return false; // Bağımlılık yükleme hatası
        }
    }

    // Modül başlatma fonksiyonunu çağırma
    if (module->init != NULL) {
        if (!module->init()) {
            return false; // Başlatma hatası
        }
    }

    // Modül yükleme mantığı (örneğin, dinamik kütüphane yükleme)
    // ... modül yükleme mantığı ...

    return true;
}

// Modül Kaldırma Fonksiyonu
void module_unload(Module* module) {
    // Modül temizleme fonksiyonunu çağırma
    if (module->cleanup != NULL) {
        module->cleanup();
    }

    // Modül kaldırma mantığı (örneğin, dinamik kütüphane kaldırma)
    // ... modül kaldırma mantığı ...

    // Modülün bağımlılıklarını kaldırma
    for (size_t i = 0; i < module->dependency_count; i++) {
        module_unload(module->dependencies[i]);
    }
}

// Modül Serbest Bırakma Fonksiyonu
void module_free(Module* module) {
    free(module->name);
    free(module->dependencies);
    free(module);
}