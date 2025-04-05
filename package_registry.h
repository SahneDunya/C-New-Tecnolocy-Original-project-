#ifndef PACKAGE_REGISTRY_H
#define PACKAGE_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>
#include "package_config.h"

// Paket Kayıt Defteri Yapısı
typedef struct {
    char* url; // Kayıt defteri URL'si veya yerel dizin yolu
} PackageRegistry;

// Paket Kayıt Defteri Oluşturma Fonksiyonu
PackageRegistry* package_registry_create(const char* url);

// Paketi Arama Fonksiyonu
PackageConfig* package_registry_search(PackageRegistry* registry, const char* package_name);

// Paketi İndirme Fonksiyonu
bool package_registry_download(PackageRegistry* registry, const char* package_name, const char* version, const char* destination_path);

// Paket Bilgilerini Alma Fonksiyonu
PackageConfig* package_registry_get_package_info(PackageRegistry* registry, const char* package_name, const char* version);

// Paket Kayıt Defterini Serbest Bırakma Fonksiyonu
void package_registry_free(PackageRegistry* registry);

#endif