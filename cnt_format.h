#ifndef CNT_FORMAT_H
#define CNT_FORMAT_H

#include <stdint.h>
#include <stdbool.h>

// CNT Dosya Başlığı Yapısı
typedef struct {
    uint32_t magicNumber; // CNT dosya kimliği (örneğin, 0x434E5400)
    uint32_t version;      // Dosya formatı sürümü
    uint32_t entryCount;   // Giriş sayısı
} CNTHeader;

// CNT Dosya Girişi Yapısı
typedef struct {
    uint32_t keyLength;   // Anahtar uzunluğu
    char* key;           // Anahtar (null ile sonlandırılmış)
    uint32_t valueLength; // Değer uzunluğu
    void* value;         // Değer
} CNTEntry;

// CNT Dosyası Oluşturma Fonksiyonu
bool CNT_Create(const char* filename, CNTHeader* header, CNTEntry* entries);

// CNT Dosyasını Okuma Fonksiyonu
bool CNT_Read(const char* filename, CNTHeader* header, CNTEntry** entries);

// CNT Dosyasını Yazma Fonksiyonu
bool CNT_Write(const char* filename, CNTHeader* header, CNTEntry* entries);

// CNT Dosyasını Kapatma Fonksiyonu
bool CNT_Close(CNTEntry* entries);

#endif