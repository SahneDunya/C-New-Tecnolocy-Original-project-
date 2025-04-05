#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_cache.h"

// Yardımcı Fonksiyonlar (Basit Önbellekleme)

static bool copy_file(const char* source_path, const char* destination_path) {
    // ... dosyayı kopyalama mantığı ...
    // Örnek olarak, basit bir dosya kopyalama komutu:
    char command[1024];
    sprintf(command, "cp %s %s", source_path, destination_path);
    return system(command) == 0;
}

static bool remove_file(const char* file_path) {
    // ... dosyayı silme mantığı ...
    // Örnek olarak, basit bir dosya silme komutu:
    char command[1024];
    sprintf(command, "rm %s", file_path);
    return system(command) == 0;
}

// Paket Önbelleği Oluşturma Fonksiyonu
PackageCache* package_cache_create(const char* cache_directory) {
    PackageCache* cache = (PackageCache*)malloc(sizeof(PackageCache));
    cache->cache_directory = strdup(cache_directory);
    return cache;
}

// Paketi Önbelleğe Alma Fonksiyonu
bool package_cache_store(PackageCache* cache, PackageConfig* config, const char* package_path) {
    // Önbellek dizinini oluşturma
    char cache_package_directory[1024];
    sprintf(cache_package_directory, "%s/%s", cache->cache_directory, config->name);
    char command[1024];
    sprintf(command, "mkdir -p %s", cache_package_directory);
    if (system(command) != 0) {
        return false;
    }

    // Paketi önbelleğe kopyalama
    char cache_package_path[1024];
    sprintf(cache_package_path, "%s/%s.tar.gz", cache_package_directory, config->version);
    return copy_file(package_path, cache_package_path);
}

// Paketi Önbellekten Alma Fonksiyonu
char* package_cache_retrieve(PackageCache* cache, PackageConfig* config) {
    // Önbellekte paketi arama
    char cache_package_path[1024];
    sprintf(cache_package_path, "%s/%s/%s.tar.gz", cache->cache_directory, config->name, config->version);
    if (access(cache_package_path, F_OK) == 0) {
        return strdup(cache_package_path);
    }
    return NULL;
}

// Paket Önbelleğini Temizleme Fonksiyonu
bool package_cache_clear(PackageCache* cache) {
    // Önbellek dizinini temizleme
    char command[1024];
    sprintf(command, "rm -rf %s/*", cache->cache_directory);
    return system(command) == 0;
}

// Paket Önbelleğini Serbest Bırakma Fonksiyonu
void package_cache_free(PackageCache* cache) {
    free(cache->cache_directory);
    free(cache);
}