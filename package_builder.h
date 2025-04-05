#ifndef PACKAGE_BUILDER_H
#define PACKAGE_BUILDER_H

#include <stdbool.h>
#include <stddef.h>
#include "package_config.h"

// Paket Oluşturucu Yapısı
typedef struct {
    char* build_directory;
    char* install_directory;
    // ... diğer yapılandırma alanları ...
} PackageBuilder;

// Paket Oluşturucu Oluşturma Fonksiyonu
PackageBuilder* package_builder_create(const char* build_directory, const char* install_directory);

// Paketi Oluşturma Fonksiyonu
bool package_builder_build(PackageBuilder* builder, PackageConfig* config);

// Paketi Kurma Fonksiyonu
bool package_builder_install(PackageBuilder* builder, PackageConfig* config);

// Paket Oluşturucuyu Serbest Bırakma Fonksiyonu
void package_builder_free(PackageBuilder* builder);

#endif