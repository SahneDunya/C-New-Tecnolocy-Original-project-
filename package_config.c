#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_config.h"

// Yardımcı Fonksiyonlar (Basit Ayrıştırma ve Oluşturma)

static char* read_string(FILE* file) {
    // ... dosyadan dize okuma ...
}

static char** read_string_array(FILE* file, size_t* count) {
    // ... dosyadan dize dizisi okuma ...
}

static void write_string(FILE* file, const char* string) {
    // ... dosyaya dize yazma ...
}

static void write_string_array(FILE* file, char** array, size_t count) {
    // ... dosyaya dize dizisi yazma ...
}

// Paket Yapılandırmasını Okuma Fonksiyonu
PackageConfig* package_config_read(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    PackageConfig* config = (PackageConfig*)malloc(sizeof(PackageConfig));
    config->name = read_string(file);
    config->version = read_string(file);
    config->dependencies = read_string_array(file, &config->dependency_count);
    config->scripts = read_string_array(file, &config->script_count);
    // ... diğer yapılandırma alanlarını oku ...

    fclose(file);
    return config;
}

// Paket Yapılandırmasını Yazma Fonksiyonu
bool package_config_write(const char* filename, PackageConfig* config) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }

    write_string(file, config->name);
    write_string(file, config->version);
    write_string_array(file, config->dependencies, config->dependency_count);
    write_string_array(file, config->scripts, config->script_count);
    // ... diğer yapılandırma alanlarını yaz ...

    fclose(file);
    return true;
}

// Paket Yapılandırmasını Serbest Bırakma Fonksiyonu
void package_config_free(PackageConfig* config) {
    free(config->name);
    free(config->version);
    // ... diğer yapılandırma alanlarını serbest bırak ...
    free(config);
}