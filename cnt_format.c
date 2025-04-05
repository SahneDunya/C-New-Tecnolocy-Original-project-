#include "cnt_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CNT Dosyası Oluşturma Fonksiyonu
bool CNT_Create(const char* filename, CNTHeader* header, CNTEntry* entries) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return false;
    }

    // Başlığı yaz
    fwrite(header, sizeof(CNTHeader), 1, file);

    // Girişleri yaz
    for (uint32_t i = 0; i < header->entryCount; i++) {
        fwrite(&entries[i].keyLength, sizeof(uint32_t), 1, file);
        fwrite(entries[i].key, entries[i].keyLength, 1, file);
        fwrite(&entries[i].valueLength, sizeof(uint32_t), 1, file);
        fwrite(entries[i].value, entries[i].valueLength, 1, file);
    }

    fclose(file);
    return true;
}

// CNT Dosyasını Okuma Fonksiyonu
bool CNT_Read(const char* filename, CNTHeader* header, CNTEntry** entries) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return false;
    }

    // Başlığı oku
    fread(header, sizeof(CNTHeader), 1, file);

    // Girişleri oku
    *entries = (CNTEntry*)malloc(sizeof(CNTEntry) * header->entryCount);
    if (*entries == NULL) {
        fclose(file);
        return false;
    }

    for (uint32_t i = 0; i < header->entryCount; i++) {
        fread(&(*entries)[i].keyLength, sizeof(uint32_t), 1, file);
        (*entries)[i].key = (char*)malloc((*entries)[i].keyLength + 1);
        if ((*entries)[i].key == NULL) {
            fclose(file);
            CNT_Close(*entries);
            return false;
        }
        fread((*entries)[i].key, (*entries)[i].keyLength, 1, file);
        (*entries)[i].key[(*entries)[i].keyLength] = '\0'; // Null ile sonlandır
        fread(&(*entries)[i].valueLength, sizeof(uint32_t), 1, file);
        (*entries)[i].value = malloc((*entries)[i].valueLength);
        if ((*entries)[i].value == NULL) {
            fclose(file);
            CNT_Close(*entries);
            return false;
        }
        fread((*entries)[i].value, (*entries)[i].valueLength, 1, file);
    }

    fclose(file);
    return true;
}

// CNT Dosyasını Yazma Fonksiyonu
bool CNT_Write(const char* filename, CNTHeader* header, CNTEntry* entries) {
    return CNT_Create(filename, header, entries); // Okuma ve yazma aynı fonksiyonu kullanır
}

// CNT Dosyasını Kapatma Fonksiyonu
bool CNT_Close(CNTEntry* entries) {
    if (entries == NULL) {
        return false;
    }

    // Girişlerin belleklerini serbest bırak
    for (uint32_t i = 0; i < ((CNTHeader*)entries - 1)->entryCount; i++) {
        free(entries[i].key);
        free(entries[i].value);
    }

    free(entries);
    return true;
}