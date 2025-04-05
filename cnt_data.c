#include "cnt_data.h"
#include <string.h>
#include <stdlib.h>

// CNT Dosyasındaki Bir Anahtara Göre Değer Bulma Fonksiyonu
void* CNT_GetValueByKey(CNTEntry* entries, uint32_t entryCount, const char* key) {
    for (uint32_t i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            return entries[i].value;
        }
    }
    return NULL;
}

// CNT Dosyasındaki Bir Anahtara Göre Giriş Bulma Fonksiyonu
CNTEntry* CNT_GetEntryByKey(CNTEntry* entries, uint32_t entryCount, const char* key) {
    for (uint32_t i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            return &entries[i];
        }
    }
    return NULL;
}

// CNT Dosyasındaki Tüm Anahtarları Alma Fonksiyonu
char** CNT_GetAllKeys(CNTEntry* entries, uint32_t entryCount) {
    char** keys = (char**)malloc(sizeof(char*) * entryCount);
    if (keys == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < entryCount; i++) {
        keys[i] = entries[i].key;
    }
    return keys;
}

// CNT Dosyasındaki Tüm Değerleri Alma Fonksiyonu
void** CNT_GetAllValues(CNTEntry* entries, uint32_t entryCount) {
    void** values = (void**)malloc(sizeof(void*) * entryCount);
    if (values == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < entryCount; i++) {
        values[i] = entries[i].value;
    }
    return values;
}

// CNT Dosyasındaki Tüm Girişleri Alma Fonksiyonu
CNTEntry* CNT_GetAllEntries(CNTEntry* entries, uint32_t entryCount) {
    return entries;
}

// CNT Dosyasındaki Giriş Sayısını Alma Fonksiyonu
uint32_t CNT_GetEntryCount(CNTEntry* entries, uint32_t entryCount) {
    return entryCount;
}

// CNT Dosyasındaki Bir Anahtarın Varlığını Kontrol Etme Fonksiyonu
bool CNT_KeyExists(CNTEntry* entries, uint32_t entryCount, const char* key) {
    for (uint32_t i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            return true;
        }
    }
    return false;
}

// CNT Dosyasındaki Bir Anahtara Yeni Bir Değer Atama Fonksiyonu
bool CNT_SetValueByKey(CNTEntry* entries, uint32_t entryCount, const char* key, void* newValue, uint32_t newValueLength) {
    for (uint32_t i = 0; i < entryCount; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            free(entries[i].value);
            entries[i].value = malloc(newValueLength);
            if (entries[i].value == NULL) {
                return false;
            }
            memcpy(entries[i].value, newValue, newValueLength);
            entries[i].valueLength = newValueLength;
            return true;
        }
    }
    return false;
}

// CNT Dosyasındaki Bir Anahtarı ve Değeri Silme Fonksiyonu
bool CNT_RemoveEntryByKey(CNTEntry* entries, uint32_t entryCount, const char* key) {
    // Silme işlemi için bellek yönetimi ve dizi kaydırma işlemleri gereklidir.
    // Basitlik için bu örnekte uygulanmamıştır.
    return false;
}

// CNT Dosyasına Yeni Bir Giriş Ekleme Fonksiyonu
bool CNT_AddEntry(CNTEntry** entries, uint32_t* entryCount, const char* key, void* value, uint32_t valueLength) {
    // Yeni giriş ekleme işlemi için bellek yönetimi ve dizi genişletme işlemleri gereklidir.
    // Basitlik için bu örnekte uygulanmamıştır.
    return false;
}