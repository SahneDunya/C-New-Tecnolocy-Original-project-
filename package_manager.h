#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

// Paket Yöneticisi Yapısı
typedef struct {
    // ... paket yöneticisi verileri ...
} PackageManager;

// Paket Yöneticisi Oluşturma Fonksiyonu
PackageManager* package_manager_create();

// Paket Yükleme Fonksiyonu
bool package_manager_install(PackageManager* manager, const char* package_name);

// Paket Kaldırma Fonksiyonu
bool package_manager_uninstall(PackageManager* manager, const char* package_name);

// Paket Güncelleme Fonksiyonu
bool package_manager_update(PackageManager* manager, const char* package_name);

// Paket Listeleme Fonksiyonu
char** package_manager_list(PackageManager* manager, size_t* count);

// Paket Yöneticisi Serbest Bırakma Fonksiyonu
void package_manager_free(PackageManager* manager);

#endif