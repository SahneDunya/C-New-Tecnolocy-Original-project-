#include "cnt_format.h"
#include "cnt_data.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Kullanım: %s <girdi.cnt> <anahtar> <yeni_değer>\n", argv[0]);
        return 1;
    }

    CNTHeader header;
    CNTEntry* entries;

    if (!CNT_Read(argv[1], &header, &entries)) {
        printf("CNT dosyası okuma hatası.\n");
        return 1;
    }

    CNTEntry* entry = CNT_GetEntryByKey(entries, header.entryCount, argv[2]);
    if (entry == NULL) {
        printf("Anahtar bulunamadı.\n");
        CNT_Close(entries);
        return 1;
    }

    free(entry->value);
    entry->value = malloc(strlen(argv[3]) + 1);
    strcpy(entry->value, argv[3]);
    entry->valueLength = strlen(argv[3]);

    if (!CNT_Write(argv[1], &header, entries)) {
        printf("CNT dosyası yazma hatası.\n");
        CNT_Close(entries);
        return 1;
    }

    CNT_Close(entries);
    return 0;
}