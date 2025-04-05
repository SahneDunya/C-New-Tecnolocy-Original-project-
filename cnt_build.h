#ifndef CNT_BUILD_H
#define CNT_BUILD_H

#include <stdbool.h>
#include <stddef.h>

// Derleme Yapısı
typedef struct BuildConfig {
    char** source_files;
    size_t source_file_count;
    char* output_file;
    char** include_paths;
    size_t include_path_count;
    char** library_paths;
    size_t library_path_count;
    char** libraries;
    size_t library_count;
    char** compiler_flags;
    size_t compiler_flag_count;
} BuildConfig;

// Derleme Yapılandırması Oluşturma Fonksiyonu
BuildConfig* build_config_create();

// Kaynak Dosyası Ekleme Fonksiyonu
bool build_config_add_source_file(BuildConfig* config, const char* source_file);

// Çıktı Dosyası Ayarlama Fonksiyonu
void build_config_set_output_file(BuildConfig* config, const char* output_file);

// Include Yolu Ekleme Fonksiyonu
bool build_config_add_include_path(BuildConfig* config, const char* include_path);

// Kütüphane Yolu Ekleme Fonksiyonu
bool build_config_add_library_path(BuildConfig* config, const char* library_path);

// Kütüphane Ekleme Fonksiyonu
bool build_config_add_library(BuildConfig* config, const char* library);

// Derleyici Bayrağı Ekleme Fonksiyonu
bool build_config_add_compiler_flag(BuildConfig* config, const char* compiler_flag);

// Derleme Fonksiyonu
bool build_run(BuildConfig* config);

// Derleme Yapılandırmasını Serbest Bırakma Fonksiyonu
void build_config_free(BuildConfig* config);

#endif