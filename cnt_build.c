#include "cnt_build.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Derleme Yapılandırması Oluşturma Fonksiyonu
BuildConfig* build_config_create() {
    BuildConfig* config = (BuildConfig*)malloc(sizeof(BuildConfig));
    if (config == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    config->source_files = NULL;
    config->source_file_count = 0;
    config->output_file = NULL;
    config->include_paths = NULL;
    config->include_path_count = 0;
    config->library_paths = NULL;
    config->library_path_count = 0;
    config->libraries = NULL;
    config->library_count = 0;
    config->compiler_flags = NULL;
    config->compiler_flag_count = 0;
    return config;
}

// Kaynak Dosyası Ekleme Fonksiyonu
bool build_config_add_source_file(BuildConfig* config, const char* source_file) {
    config->source_files = (char**)realloc(config->source_files, sizeof(char*) * (config->source_file_count + 1));
    if (config->source_files == NULL) {
        return false; // Bellek ayırma hatası
    }
    config->source_files[config->source_file_count] = strdup(source_file);
    config->source_file_count++;
    return true;
}

// Çıktı Dosyası Ayarlama Fonksiyonu
void build_config_set_output_file(BuildConfig* config, const char* output_file) {
    config->output_file = strdup(output_file);
}

// Include Yolu Ekleme Fonksiyonu
bool build_config_add_include_path(BuildConfig* config, const char* include_path) {
    config->include_paths = (char**)realloc(config->include_paths, sizeof(char*) * (config->include_path_count + 1));
    if (config->include_paths == NULL) {
        return false; // Bellek ayırma hatası
    }
    config->include_paths[config->include_path_count] = strdup(include_path);
    config->include_path_count++;
    return true;
}

// Kütüphane Yolu Ekleme Fonksiyonu
bool build_config_add_library_path(BuildConfig* config, const char* library_path) {
    config->library_paths = (char**)realloc(config->library_paths, sizeof(char*) * (config->library_path_count + 1));
    if (config->library_paths == NULL) {
        return false; // Bellek ayırma hatası
    }
    config->library_paths[config->library_path_count] = strdup(library_path);
    config->library_path_count++;
    return true;
}

// Kütüphane Ekleme Fonksiyonu
bool build_config_add_library(BuildConfig* config, const char* library) {
    config->libraries = (char**)realloc(config->libraries, sizeof(char*) * (config->library_count + 1));
    if (config->libraries == NULL) {
        return false; // Bellek ayırma hatası
    }
    config->libraries[config->library_count] = strdup(library);
    config->library_count++;
    return true;
}

// Derleyici Bayrağı Ekleme Fonksiyonu
bool build_config_add_compiler_flag(BuildConfig* config, const char* compiler_flag) {
    config->compiler_flags = (char**)realloc(config->compiler_flags, sizeof(char*) * (config->compiler_flag_count + 1));
    if (config->compiler_flags == NULL) {
        return false; // Bellek ayırma hatası
    }
    config->compiler_flags[config->compiler_flag_count] = strdup(compiler_flag);
    config->compiler_flag_count++;
    return true;
}

// Derleme Fonksiyonu
bool build_run(BuildConfig* config) {
    // Derleme komutunu oluşturma
    // ... derleme komutu oluşturma mantığı ...

    // Basit bir örnek: gcc kullanarak derleme
    char command[1024];
    sprintf(command, "gcc");

    // Kaynak dosyaları ekleme
    for (size_t i = 0; i < config->source_file_count; i++) {
        strcat(command, " ");
        strcat(command, config->source_files[i]);
    }

    // Include yollarını ekleme
    for (size_t i = 0; i < config->include_path_count; i++) {
        strcat(command, " -I");
        strcat(command, config->include_paths[i]);
    }

    // Kütüphane yollarını ekleme
    for (size_t i = 0; i < config->library_path_count; i++) {
        strcat(command, " -L");
        strcat(command, config->library_paths[i]);
    }

    // Kütüphaneleri ekleme
    for (size_t i = 0; i < config->library_count; i++) {
        strcat(command, " -l");
        strcat(command, config->libraries[i]);
    }

    // Derleyici bayraklarını ekleme
    for (size_t i = 0; i < config->compiler_flag_count; i++) {
        strcat(command, " ");
        strcat(command, config->compiler_flags[i]);
    }

    // Çıktı dosyasını ekleme
    if (config->output_file != NULL) {
        strcat(command, " -o ");
        strcat(command, config->output_file);
    }

    // Derleme komutunu çalıştırma
    int result = system(command);
    if (result != 0) {
        return false; // Derleme hatası
    }

    return true;
}

// Derleme Yapılandırmasını Serbest Bırakma Fonksiyonu
void build_config_free(BuildConfig* config) {
    for (size_t i = 0; i < config->source_file_count; i++) {
        free(config->source_files[i]);
    }
    free(config->source_files);
    free(config->output_file);
    for (size_t i = 0; i < config->include_path_count; i++) {
        free(config->include_paths[i]);
    }
    free(config->include_paths);
    for (size_t i = 0; i < config->library_path_count; i++) {
        free(config->library_paths[i]);
    }
    free(config->library_paths);
    for (size_t i = 0; i < config->libraries) 
}