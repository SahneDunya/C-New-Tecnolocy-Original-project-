#ifndef LIBRARY_CONFIG_H
#define LIBRARY_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include "library_selector.h" // Kütüphane seçim makroları için

// Kütüphane Yapılandırma Yapısı
typedef struct {
    int default_library; // Varsayılan kütüphane
    // ... diğer yapılandırma seçenekleri ...
} LibraryConfig;

// Kütüphane Yapılandırmasını Okuma Fonksiyonu
LibraryConfig* library_config_read(const char* filename);

// Kütüphane Yapılandırmasını Yazma Fonksiyonu
bool library_config_write(const char* filename, LibraryConfig* config);

// Kütüphane Yapılandırmasını Serbest Bırakma Fonksiyonu
void library_config_free(LibraryConfig* config);

#endif