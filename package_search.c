#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_search.h"

// Yardımcı Fonksiyonlar (Basit Arama)

static bool package_matches_query(PackageConfig* config, const char* query) {
    // ... paketin sorguyla eşleşip eşleşmediğini kontrol etme mantığı ...
    // Örnek olarak, basit bir dize arama:
    if (strstr(config->name, query) != NULL || strstr(config->version, query) != NULL) {
        return true;
    }
    return false;
}

// Paket Arama Oluşturma Fonksiyonu
PackageSearch* package_search_create(PackageRegistry* registry) {
    PackageSearch* search = (PackageSearch*)malloc(sizeof(PackageSearch));
    search->registry = registry;
    return search;
}

// Paketleri Arama Fonksiyonu
PackageConfig** package_search_packages(PackageSearch* search, const char* query, size_t* result_count) {
    // Kayıt defterindeki tüm paketleri listeleme
    // ... kayıt defterindeki tüm paketleri listeleme mantığı ...

    // Listelenen paketleri sorguyla eşleştirme
    PackageConfig** matching_packages = NULL;
    size_t count = 0;
    // ... tüm paketleri listeleme mantığı ...
    PackageConfig** all_packages = package_registry_get_all_packages(search->registry);
    if (all_packages == NULL) {
        return NULL;
    }
    size_t all_packages_size = 0; // Tüm paketleri sayan bir fonksiyon gerekir.
    all_packages_size = package_registry_get_all_packages_size(search->registry);

    for (size_t i = 0; i < all_packages_size; i++) {
        if (package_matches_query(all_packages[i], query)) {
            matching_packages = (PackageConfig**)realloc(matching_packages, sizeof(PackageConfig*) * (count + 1));
            matching_packages[count] = all_packages[i];
            count++;
        }
    }

    *result_count = count;
    return matching_packages;
}

// Paket Aramayı Serbest Bırakma Fonksiyonu
void package_search_free(PackageSearch* search) {
    free(search);
}