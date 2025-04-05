#ifndef PACKAGE_CONFIG_H
#define PACKAGE_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

// Paket Yapılandırma Yapısı
typedef struct {
    char* name;
    char* version;
    char** dependencies;
    size_t dependency_count;
    char** scripts;
    size_t script_count;
    // ... diğer yapılandırma alanları ...
} PackageConfig;

// Paket Yapılandırmasını Okuma Fonksiyonu
PackageConfig* package_config_read(const char* filename);

// Paket Yapılandırmasını Yazma Fonksiyonu
bool package_config_write(const char* filename, PackageConfig* config);

// Paket Yapılandırmasını Serbest Bırakma Fonksiyonu
void package_config_free(PackageConfig* config);

#endif