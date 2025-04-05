#ifndef DEPENDENCY_RESOLVER_H
#define DEPENDENCY_RESOLVER_H

#include <stdbool.h>
#include <stddef.h>

// Paket Yapısı
typedef struct {
    char* name;
    char** dependencies;
} Package;

// Bağımlılıkları Çözme Fonksiyonu
Package** resolve_dependencies(Package* package, Package** available_packages, size_t available_packages_count, size_t* resolved_packages_count);

// Paket Bağımlılıklarını Serbest Bırakma Fonksiyonu
void free_package_dependencies(Package** resolved_packages, size_t resolved_packages_count);

#endif