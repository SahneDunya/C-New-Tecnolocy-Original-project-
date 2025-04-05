#include <stdio.h>
#include "cnt_file_writer.h"

void write_cnt_file(const char* filename, const char* key, const char* value) {
    FILE* file = fopen(filename, "a"); // Ekleme modunda aç
    if (file == NULL) {
        perror("Dosya açma hatası");
        return;
    }

    fprintf(file, "%s=%s\n", key, value);
    fclose(file);
}