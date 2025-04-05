#ifndef PACKAGE_CACHE_H
#define PACKAGE_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include "package_config.h"

// Paket Önbelleği Yapısı
typedef struct {
    char* cache_directory;
    // ... diğer yapılandırma alanları ...
} PackageCache;

// Paket Önbelleği Oluşturma Fonksiyonu
PackageCache* package_cache_create(const char* cache_directory);

// Paketi Önbelleğe Alma Fonksiyonu
bool package_cache_store(PackageCache* cache, PackageConfig* config, const char* package_path);

// Paketi Önbellekten Alma Fonksiyonu
char* package_cache_retrieve(PackageCache* cache, PackageConfig* config);

// Paket Önbelleğini Temizleme Fonksiyonu
bool package_cache_clear(PackageCache* cache);

// Paket Önbelleğini Serbest Bırakma Fonksiyonu
void package_cache_free(PackageCache* cache);

#endif