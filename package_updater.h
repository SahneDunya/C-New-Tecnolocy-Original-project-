#ifndef PACKAGE_UPDATER_H
#define PACKAGE_UPDATER_H

#include <stdbool.h>
#include <stddef.h>
#include "package_config.h"
#include "package_registry.h"

// Paket Güncelleyici Yapısı
typedef struct {
    PackageRegistry* registry;
    char* install_directory;
    // ... diğer yapılandırma alanları ...
} PackageUpdater;

// Paket Güncelleyici Oluşturma Fonksiyonu
PackageUpdater* package_updater_create(PackageRegistry* registry, const char* install_directory);

// Paketi Güncelleme Fonksiyonu
bool package_updater_update(PackageUpdater* updater, PackageConfig* config);

// Tüm Paketleri Güncelleme Fonksiyonu
bool package_updater_update_all(PackageUpdater* updater);

// Paket Güncelleyiciyi Serbest Bırakma Fonksiyonu
void package_updater_free(PackageUpdater* updater);

#endif