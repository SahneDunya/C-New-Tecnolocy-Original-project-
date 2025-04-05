#ifndef CPL_H
#define CPL_H

#include <stdbool.h>
#include <stddef.h>

// Paket Yapısı
typedef struct {
    char* name;
    char* version;
    char** dependencies;
    // ... diğer paket bilgileri ...
} Package;

// Paket Listesi Yapısı
typedef struct {
    Package** packages;
    size_t count;
} PackageList;

// Paket Listesi Oluşturma Fonksiyonu
PackageList* cpl_create_package_list();

// Paket Listesine Paket Ekleme Fonksiyonu
bool cpl_add_package(PackageList* package_list, Package* package);

// Paket Listesinden Paket Kaldırma Fonksiyonu
bool cpl_remove_package(PackageList* package_list, const char* package_name);

// Paket Listesindeki Paketleri Listeleme Fonksiyonu
void cpl_list_packages(PackageList* package_list);

// Paket Bağımlılıklarını Çözme Fonksiyonu
PackageList* cpl_resolve_dependencies(PackageList* package_list);

// Paket Listesini Serbest Bırakma Fonksiyonu
void cpl_free_package_list(PackageList* package_list);

#endif