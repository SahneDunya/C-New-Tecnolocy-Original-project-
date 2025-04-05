#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_file_reader.h"

#define MAX_LINE_LENGTH 256

void read_cnt_file(const char* filename, void (*process_line)(const char*, const char*)) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Dosya açma hatası");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Satırı işle
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "\n");

        if (key && value) {
            process_line(key, value);
        }
    }

    fclose(file);
}