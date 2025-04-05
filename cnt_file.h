#ifndef CNT_FILE_H
#define CNT_FILE_H

#include <stdio.h> // Temel C dosya işlemleri için
#include <stdbool.h>

// CNT Dosya Yapısı (C'deki FILE yapısını soyutlar)
typedef struct CNT_File {
    FILE* file_ptr;
    bool is_open;
} CNT_File;

// Dosya açma fonksiyonu
CNT_File* cnt_fopen(const char* filename, const char* mode);

// Dosya kapama fonksiyonu
bool cnt_fclose(CNT_File* file);

// Dosyadan bir satır okuma fonksiyonu (belleği dinamik olarak ayırır)
char* cnt_fread_line(CNT_File* file);

// Dosyadan belirtilen sayıda byte okuma fonksiyonu
size_t cnt_fread_bytes(CNT_File* file, void* buffer, size_t size);

// Dosyaya bir satır yazma fonksiyonu
bool cnt_fwrite_line(CNT_File* file, const char* line);

// Dosyaya belirtilen sayıda byte yazma fonksiyonu
size_t cnt_fwrite_bytes(CNT_File* file, const void* buffer, size_t size);

// Dosya konumunu ayarlama fonksiyonu
int cnt_fseek(CNT_File* file, long offset, int whence);

// Dosyanın geçerli konumunu döndüren fonksiyon
long cnt_ftell(CNT_File* file);

// Dosyanın sonuna ulaşıldı mı kontrol eden fonksiyon
bool cnt_feof(CNT_File* file);

// Dosyada bir hata oluştu mu kontrol eden fonksiyon
bool cnt_ferror(CNT_File* file);

#endif