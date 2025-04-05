#ifndef PACKAGE_INSTALLER_H
#define PACKAGE_INSTALLER_H

#include <stdbool.h>
#include <stddef.h>
#include "package_config.h"

// Paket Yükleyici Yapısı
typedef struct {
    char* install_directory;
    // ... diğer yapılandırma alanları ...
} PackageInstaller;

// Paket Yükleyici Oluşturma Fonksiyonu
PackageInstaller* package_installer_create(const char* install_directory);

// Paketi Yükleme Fonksiyonu
bool package_installer_install(PackageInstaller* installer, PackageConfig* config, const char* package_path);

// Paketleri Kaldırma Fonksiyonu
bool package_installer_uninstall(PackageInstaller* installer, PackageConfig* config);

// Paket Yükleyiciyi Serbest Bırakma Fonksiyonu
void package_installer_free(PackageInstaller* installer);

#endif