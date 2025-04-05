#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dependency_resolver.h"

// Bağımlılıkları Çözme Fonksiyonu
Package** resolve_dependencies(Package* package, Package** available_packages, size_t available_packages_count, size_t* resolved_packages_count) {
    Package** resolved_packages = NULL;
    size_t resolved_packages_index = 0;

    // Paketin kendisini ekle
    resolved_packages = (Package**)realloc(resolved_packages, sizeof(Package*) * (resolved_packages_index + 1));
    resolved_packages[resolved_packages_index++] = package;

    // Paketin bağımlılıklarını çöz
    if (package->dependencies != NULL) {
        for (size_t i = 0; package->dependencies[i] != NULL; i++) {
            char* dependency_name = package->dependencies[i];

            // Bağımlılığı kullanılabilir paketlerde ara
            Package* dependency = NULL;
            for (size_t j = 0; j < available_packages_count; j++) {
                if (strcmp(available_packages[j]->name, dependency_name) == 0) {
                    dependency = available_packages[j];
                    break;
                }
            }

            // Bağımlılık bulunamazsa hata ver
            if (dependency == NULL) {
                fprintf(stderr, "Hata: Bağımlılık bulunamadı: %s\n", dependency_name);
                free_package_dependencies(resolved_packages, resolved_packages_index);
                return NULL;
            }

            // Bağımlılığı çözülmüş paketlere ekle (tekrarlamayı önle)
            bool already_resolved = false;
            for (size_t j = 0; j < resolved_packages_index; j++) {
                if (resolved_packages[j] == dependency) {
                    already_resolved = true;
                    break;
                }
            }
            if (!already_resolved) {
                resolved_packages = (Package**)realloc(resolved_packages, sizeof(Package*) * (resolved_packages_index + 1));
                resolved_packages[resolved_packages_index++] = dependency;
            }
        }
    }

    *resolved_packages_count = resolved_packages_index;
    return resolved_packages;
}

// Paket Bağımlılıklarını Serbest Bırakma Fonksiyonu
void free_package_dependencies(Package** resolved_packages, size_t resolved_packages_count) {
    if (resolved_packages != NULL) {
        for (size_t i = 0; i < resolved_packages_count; i++) {
            // Paketlerin kendileri serbest bırakılmaz, sadece bağımlılık listesi serbest bırakılır
            if (resolved_packages[i]->dependencies != NULL) {
                for (size_t j = 0; resolved_packages[i]->dependencies[j] != NULL; j++) {
                    free(resolved_packages[i]->dependencies[j]);
                }
                free(resolved_packages[i]->dependencies);
            }
        }
        free(resolved_packages);
    }
}