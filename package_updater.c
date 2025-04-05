#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_updater.h"
#include "package_installer.h" // Paket yükleme için

// Yardımcı Fonksiyonlar (Basit Güncelleme)

static bool download_package(PackageUpdater* updater, PackageConfig* config, const char* destination_path) {
    // ... paketi indirme mantığı ...
    // Örnek olarak, package_registry_download fonksiyonunu kullanma:
    return package_registry_download(updater->registry, config->name, config->version, destination_path);
}

// Paket Güncelleyici Oluşturma Fonksiyonu
PackageUpdater* package_updater_create(PackageRegistry* registry, const char* install_directory) {
    PackageUpdater* updater = (PackageUpdater*)malloc(sizeof(PackageUpdater));
    updater->registry = registry;
    updater->install_directory = strdup(install_directory);
    return updater;
}

// Paketi Güncelleme Fonksiyonu
bool package_updater_update(PackageUpdater* updater, PackageConfig* config) {
    // En son sürümü kontrol etme
    PackageConfig* latest_config = package_registry_search(updater->registry, config->name);
    if (latest_config == NULL) {
        return false;
    }

    // Güncelleme gerekip gerekmediğini kontrol etme
    if (strcmp(config->version, latest_config->version) == 0) {
        printf("%s zaten en son sürümde.\n", config->name);
        package_config_free(latest_config);
        return true;
    }

    // Paketi indirme
    char package_path[1024];
    sprintf(package_path, "%s/%s.tar.gz", updater->install_directory, config->name);
    if (!download_package(updater, latest_config, package_path)) {
        package_config_free(latest_config);
        return false;
    }

    // Paketi yükleme
    PackageInstaller* installer = package_installer_create(updater->install_directory);
    bool result = package_installer_install(installer, latest_config, package_path);
    package_installer_free(installer);
    package_config_free(latest_config);
    return result;
}

// Tüm Paketleri Güncelleme Fonksiyonu
bool package_updater_update_all(PackageUpdater* updater) {
    // Yüklü paketleri listeleme
    // ... yüklü paketleri listeleme mantığı ...

    // Her paketi güncelleme
    // ... her paketi güncelleme mantığı ...

    return true;
}

// Paket Güncelleyiciyi Serbest Bırakma Fonksiyonu
void package_updater_free(PackageUpdater* updater) {
    free(updater->install_directory);
    free(updater);
}