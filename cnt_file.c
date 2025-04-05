#include "cnt_file.h"
#include <stdlib.h>
#include <string.h>

CNT_File* cnt_fopen(const char* filename, const char* mode) {
    FILE* fp = fopen(filename, mode);
    if (fp == NULL) {
        return NULL;
    }
    CNT_File* cnt_file = (CNT_File*)malloc(sizeof(CNT_File));
    if (cnt_file == NULL) {
        fclose(fp);
        return NULL;
    }
    cnt_file->file_ptr = fp;
    cnt_file->is_open = true;
    return cnt_file;
}

bool cnt_fclose(CNT_File* file) {
    if (file == NULL || !file->is_open) {
        return true; // Zaten kapalı veya geçersiz
    }
    if (fclose(file->file_ptr) == 0) {
        file->is_open = false;
        free(file);
        return true;
    } else {
        return false;
    }
}

char* cnt_fread_line(CNT_File* file) {
    if (file == NULL || !file->is_open) {
        return NULL;
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, file->file_ptr);
    if (read != -1) {
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        return line;
    } else {
        free(line);
        return NULL;
    }
}

size_t cnt_fread_bytes(CNT_File* file, void* buffer, size_t size) {
    if (file == NULL || !file->is_open || buffer == NULL || size == 0) {
        return 0;
    }
    return fread(buffer, 1, size, file->file_ptr);
}

bool cnt_fwrite_line(CNT_File* file, const char* line) {
    if (file == NULL || !file->is_open || line == NULL) {
        return false;
    }
    if (fputs(line, file->file_ptr) != EOF) {
        if (fputc('\n', file->file_ptr) != EOF) {
            return true;
        }
    }
    return false;
}

size_t cnt_fwrite_bytes(CNT_File* file, const void* buffer, size_t size) {
    if (file == NULL || !file->is_open || buffer == NULL || size == 0) {
        return 0;
    }
    return fwrite(buffer, 1, size, file->file_ptr);
}

int cnt_fseek(CNT_File* file, long offset, int whence) {
    if (file == NULL || !file->is_open) {
        return -1; // Geçersiz dosya
    }
    return fseek(file->file_ptr, offset, whence);
}

long cnt_ftell(CNT_File* file) {
    if (file == NULL || !file->is_open) {
        return -1; // Geçersiz dosya
    }
    return ftell(file->file_ptr);
}

bool cnt_feof(CNT_File* file) {
    if (file == NULL || !file->is_open) {
        return true; // Geçersiz dosya sonundaymış gibi davran
    }
    return feof(file->file_ptr) != 0;
}

bool cnt_ferror(CNT_File* file) {
    if (file == NULL || !file->is_open) {
        return true; // Geçersiz dosyada hata var gibi davran
    }
    return ferror(file->file_ptr) != 0;
}