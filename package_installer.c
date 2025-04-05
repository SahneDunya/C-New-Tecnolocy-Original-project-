#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_installer.h"

// Yardımcı Fonksiyonlar (Basit Yükleme ve Kaldırma)

static bool extract_package(const char* package_path, const char* install_directory) {
    // ... paketi çıkarma mantığı ...
    // Örnek olarak, basit bir arşiv çıkarma komutu:
    char command[1024];
    sprintf(command, "tar -xzvf %s -C %s", package_path, install_directory);
    return system(command) == 0;
}

static bool run_script(const char* script) {
    // ... betiği çalıştırma ve sonucunu kontrol etme ...
    return system(script) == 0;
}

static bool remove_directory(const char* directory) {
    // ... dizini ve içeriğini silme mantığı ...
    // Örnek olarak, basit bir dizin silme komutu:
    char command[1024];
    sprintf(command, "rm -rf %s", directory);
    return system(command) == 0;
}

// Paket Yükleyici Oluşturma Fonksiyonu
PackageInstaller* package_installer_create(const char* install_directory) {
    PackageInstaller* installer = (PackageInstaller*)malloc(sizeof(PackageInstaller));
    installer->install_directory = strdup(install_directory);
    return installer;
}

// Paketi Yükleme Fonksiyonu
bool package_installer_install(PackageInstaller* installer, PackageConfig* config, const char* package_path) {
    // Paketi çıkarma
    if (!extract_package(package_path, installer->install_directory)) {
        return false;
    }

    // Yükleme betiklerini çalıştırma
    for (size_t i = 0; i < config->script_count; i++) {
        if (!run_script(config->scripts[i])) {
            return false;
        }
    }

    // Bağımlılıkları kontrol etme ve yükleme
    // ... bağımlılıkları kontrol etme ve yükleme mantığı ...

    return true;
}

// Paketleri Kaldırma Fonksiyonu
bool package_installer_uninstall(PackageInstaller* installer, PackageConfig* config) {
    // Kaldırma betiklerini çalıştırma
    for (size_t i = 0; i < config->script_count; i++) {
        // Kaldırma betiği çalıştırma mantığı
        // ... kaldırma betiği çalıştırma ...
    }

    // Paket dosyalarını silme
    char package_directory[1024];
    sprintf(package_directory, "%s/%s", installer->install_directory, config->name);
    if (!remove_directory(package_directory)) {
        return false;
    }

    return true;
}

// Paket Yükleyiciyi Serbest Bırakma Fonksiyonu
void package_installer_free(PackageInstaller* installer) {
    free(installer->install_directory);
    free(installer);
}