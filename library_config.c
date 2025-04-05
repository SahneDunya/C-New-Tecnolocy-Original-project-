#include "library_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kütüphane Yapılandırmasını Okuma Fonksiyonu
LibraryConfig* library_config_read(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL; // Dosya açılamadı
    }

    LibraryConfig* config = (LibraryConfig*)malloc(sizeof(LibraryConfig));
    if (config == NULL) {
        fclose(file);
        return NULL; // Bellek ayırma hatası
    }

    // Yapılandırma seçeneklerini dosyadan okuma
    fscanf(file, "default_library=%d\n", &config->default_library);
    // ... diğer yapılandırma seçeneklerini okuma ...

    fclose(file);
    return config;
}

// Kütüphane Yapılandırmasını Yazma Fonksiyonu
bool library_config_write(const char* filename, LibraryConfig* config) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return false; // Dosya açılamadı
    }

    // Yapılandırma seçeneklerini dosyaya yazma
    fprintf(file, "default_library=%d\n", config->default_library);
    // ... diğer yapılandırma seçeneklerini yazma ...

    fclose(file);
    return true;
}

// Kütüphane Yapılandırmasını Serbest Bırakma Fonksiyonu
void library_config_free(LibraryConfig* config) {
    free(config);
}