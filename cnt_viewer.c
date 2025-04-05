#include "cnt_format.h"
#include "cnt_data.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Kullanım: %s <girdi.cnt>\n", argv[0]);
        return 1;
    }

    CNTHeader header;
    CNTEntry* entries;

    if (!CNT_Read(argv[1], &header, &entries)) {
        printf("CNT dosyası okuma hatası.\n");
        return 1;
    }

    printf("CNT Dosyası İçeriği:\n");
    for (uint32_t i = 0; i < header.entryCount; i++) {
        printf("Anahtar: %s, Değer: %.*s\n", entries[i].key, entries[i].valueLength, (char*)entries[i].value);
    }

    CNT_Close(entries);
    return 0;
}