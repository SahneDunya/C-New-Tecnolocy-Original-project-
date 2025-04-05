#include "cnt_format.h"
#include "cnt_data.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Kullanım: %s <girdi.cnt> <çıktı.txt>\n", argv[0]);
        return 1;
    }

    CNTHeader header;
    CNTEntry* entries;

    if (!CNT_Read(argv[1], &header, &entries)) {
        printf("CNT dosyası okuma hatası.\n");
        return 1;
    }

    FILE* outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        printf("Çıktı dosyası açma hatası.\n");
        CNT_Close(entries);
        return 1;
    }

    fprintf(outputFile, "CNT Dosyası İçeriği:\n");
    for (uint32_t i = 0; i < header.entryCount; i++) {
        fprintf(outputFile, "Anahtar: %s, Değer: %.*s\n", entries[i].key, entries[i].valueLength, (char*)entries[i].value);
    }

    fclose(outputFile);
    CNT_Close(entries);
    return 0;
}