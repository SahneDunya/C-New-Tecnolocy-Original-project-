#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpl.h"

// Paket Listesi Oluşturma Fonksiyonu
PackageList* cpl_create_package_list() {
    PackageList* package_list = (PackageList*)malloc(sizeof(PackageList));
    package_list->packages = NULL;
    package_list->count = 0;
    return package_list;
}

// Paket Listesine Paket Ekleme Fonksiyonu
bool cpl_add_package(PackageList* package_list, Package* package) {
    package_list->packages = (Package**)realloc(package_list->packages, sizeof(Package*) * (package_list->count + 1));
    if (package_list->packages == NULL) {
        return false;
    }
    package_list->packages[package_list->count] = package;
    package_list->count++;
    return true;
}

// Paket Listesinden Paket Kaldırma Fonksiyonu
bool cpl_remove_package(PackageList* package_list, const char* package_name) {
    for (size_t i = 0; i < package_list->count; i++) {
        if (strcmp(package_list->packages[i]->name, package_name) == 0) {
            // Paketi kaldır
            free(package_list->packages[i]);
            for (size_t j = i; j < package_list->count - 1; j++) {
                package_list->packages[j] = package_list->packages[j + 1];
            }
            package_list->count--;
            return true;
        }
    }
    return false;
}

// Paket Listesindeki Paketleri Listeleme Fonksiyonu
void cpl_list_packages(PackageList* package_list) {
    printf("Yüklü Paketler:\n");
    for (size_t i = 0; i < package_list->count; i++) {
        printf("- %s (%s)\n", package_list->packages[i]->name, package_list->packages[i]->version);
    }
}

// Paket Bağımlılıklarını Çözme Fonksiyonu
PackageList* cpl_resolve_dependencies(PackageList* package_list) {
    // ... paket bağımlılıklarını çözme mantığı ...
    // ... bu örnekte, bağımlılık çözme mantığı basitçe paket listesini döndürür ...
    return package_list;
}

// Paket Listesini Serbest Bırakma Fonksiyonu
void cpl_free_package_list(PackageList* package_list) {
    for (size_t i = 0; i < package_list->count; i++) {
        free(package_list->packages[i]);
    }
    free(package_list->packages);
    free(package_list);
}