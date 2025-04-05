#ifndef CNT_DATA_H
#define CNT_DATA_H

#include "cnt_format.h"
#include <stdint.h>
#include <stdbool.h>

// CNT Dosyasındaki Bir Anahtara Göre Değer Bulma Fonksiyonu
void* CNT_GetValueByKey(CNTEntry* entries, uint32_t entryCount, const char* key);

// CNT Dosyasındaki Bir Anahtara Göre Giriş Bulma Fonksiyonu
CNTEntry* CNT_GetEntryByKey(CNTEntry* entries, uint32_t entryCount, const char* key);

// CNT Dosyasındaki Tüm Anahtarları Alma Fonksiyonu
char** CNT_GetAllKeys(CNTEntry* entries, uint32_t entryCount);

// CNT Dosyasındaki Tüm Değerleri Alma Fonksiyonu
void** CNT_GetAllValues(CNTEntry* entries, uint32_t entryCount);

// CNT Dosyasındaki Tüm Girişleri Alma Fonksiyonu
CNTEntry* CNT_GetAllEntries(CNTEntry* entries, uint32_t entryCount);

// CNT Dosyasındaki Giriş Sayısını Alma Fonksiyonu
uint32_t CNT_GetEntryCount(CNTEntry* entries, uint32_t entryCount);

// CNT Dosyasındaki Bir Anahtarın Varlığını Kontrol Etme Fonksiyonu
bool CNT_KeyExists(CNTEntry* entries, uint32_t entryCount, const char* key);

// CNT Dosyasındaki Bir Anahtara Yeni Bir Değer Atama Fonksiyonu
bool CNT_SetValueByKey(CNTEntry* entries, uint32_t entryCount, const char* key, void* newValue, uint32_t newValueLength);

// CNT Dosyasındaki Bir Anahtarı ve Değeri Silme Fonksiyonu
bool CNT_RemoveEntryByKey(CNTEntry* entries, uint32_t entryCount, const char* key);

// CNT Dosyasına Yeni Bir Giriş Ekleme Fonksiyonu
bool CNT_AddEntry(CNTEntry** entries, uint32_t* entryCount, const char* key, void* value, uint32_t valueLength);

#endif