#ifndef PACKAGE_SEARCH_H
#define PACKAGE_SEARCH_H

#include <stdbool.h>
#include <stddef.h>
#include "package_registry.h"
#include "package_config.h"

// Paket Arama Yapısı
typedef struct {
    PackageRegistry* registry;
    // ... diğer yapılandırma alanları ...
} PackageSearch;

// Paket Arama Oluşturma Fonksiyonu
PackageSearch* package_search_create(PackageRegistry* registry);

// Paketleri Arama Fonksiyonu
PackageConfig** package_search_packages(PackageSearch* search, const char* query, size_t* result_count);

// Paket Aramayı Serbest Bırakma Fonksiyonu
void package_search_free(PackageSearch* search);

#endif