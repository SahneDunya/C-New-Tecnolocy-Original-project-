#ifndef CNT_FILESYSTEM_H
#define CNT_FILESYSTEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Dosya Bilgisi Yapısı
typedef struct {
    char* name;
    uint64_t size;
    uint64_t creation_time;
    uint64_t modification_time;
    bool is_directory;
} FileInfo;

// Dosya/Dizin Oluşturma Fonksiyonu
bool create_file_or_directory(const char* path, bool is_directory);

// Dosya/Dizin Silme Fonksiyonu
bool delete_file_or_directory(const char* path);

// Dosya/Dizin Yeniden Adlandırma Fonksiyonu
bool rename_file_or_directory(const char* old_path, const char* new_path);

// Dosya/Dizin Bilgisi Alma Fonksiyonu
FileInfo* get_file_info(const char* path);

// Dizin İçeriğini Listeleme Fonksiyonu
FileInfo** list_directory(const char* path, size_t* count);

// Dosya Okuma/Yazma Fonksiyonları (Basit örnek)
char* read_file(const char* path, size_t* size);
bool write_file(const char* path, const char* data, size_t size);

#endif