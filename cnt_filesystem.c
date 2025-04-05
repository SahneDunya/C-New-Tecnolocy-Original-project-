#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_filesystem.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

// Platforma özgü kodlar (örnek olarak Windows ve Linux)

#ifdef _WIN32
bool create_file_or_directory(const char* path, bool is_directory) {
    if (is_directory) {
        return CreateDirectoryA(path, NULL);
    } else {
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }
        CloseHandle(hFile);
        return true;
    }
}

bool delete_file_or_directory(const char* path) {
    if (GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY) {
        return RemoveDirectoryA(path);
    } else {
        return DeleteFileA(path);
    }
}

bool rename_file_or_directory(const char* old_path, const char* new_path) {
    return MoveFileA(old_path, new_path);
}

FileInfo* get_file_info(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA file_data;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &file_data)) {
        return NULL;
    }

    FileInfo* info = (FileInfo*)malloc(sizeof(FileInfo));
    info->name = strdup(path); // Basit örnek, gerçek uygulamada dosya adını ayıklamanız gerekebilir
    info->size = (uint64_t)file_data.nFileSizeHigh << 32 | file_data.nFileSizeLow;
    info->creation_time = (uint64_t)file_data.ftCreationTime.dwHighDateTime << 32 | file_data.ftCreationTime.dwLowDateTime;
    info->modification_time = (uint64_t)file_data.ftLastWriteTime.dwHighDateTime << 32 | file_data.ftLastWriteTime.dwLowDateTime;
    info->is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return info;
}

FileInfo** list_directory(const char* path, size_t* count) {
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFileA(strcat(strdup(path), "\\*"), &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    FileInfo** files = NULL;
    size_t file_count = 0;
    do {
        if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
            files = (FileInfo**)realloc(files, sizeof(FileInfo*) * (file_count + 1));
            files[file_count] = (FileInfo*)malloc(sizeof(FileInfo));
            files[file_count]->name = strdup(find_data.cFileName);
            files[file_count]->is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            file_count++;
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    *count = file_count;
    return files;
}

char* read_file(const char* path, size_t* size) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    DWORD file_size = GetFileSize(hFile, NULL);
    char* data = (char*)malloc(file_size + 1);
    ReadFile(hFile, data, file_size, NULL, NULL);
    CloseHandle(hFile);

    *size = file_size;
    data[file_size] = '\0';
    return data;
}

bool write_file(const char* path, const char* data, size_t size) {
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    WriteFile(hFile, data, size, NULL, NULL);
    CloseHandle(hFile);
    return true;
}

#else

bool create_file_or_directory(const char* path, bool is_directory) {
    if (is_directory) {
        return mkdir(path, 0755) == 0;
    } else {
        FILE* file = fopen(path, "w");
        if (file == NULL) {
            return false;
        }
        fclose(file);
        return true;
    }
}

bool delete_file_or_directory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        return rmdir(path) == 0;
    } else {
        return unlink(path) == 0;
    }
}

bool rename_file_or_directory(const char* old_path, const char* new_path) {
    return rename(old_path, new_path) == 0;
}

FileInfo* get_file_info(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return NULL;
    }

    FileInfo* info = (FileInfo*)malloc(sizeof(FileInfo));
    info->name = strdup(path); // Basit örnek, gerçek uygulamada dosya adını ayıklamanız gerekebilir
    info->size = statbuf.st_size;
    info->creation_time = statbuf.st_ctime;
    info->modification_time = statbuf.st_mtime;
    info->is_directory = S_ISDIR(statbuf.st_mode);
    return info;
}

FileInfo** list_directory(const char* path, size_t* count) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        return NULL;
    }

    FileInfo** files = NULL;
    size_t file_count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            files = (FileInfo**)realloc(files, sizeof(FileInfo*) * (file_count + 1));
            files[file_count] = (FileInfo*)malloc(sizeof(FileInfo));
            files[file_count]->name = strdup(entry->d_name);
            files[file_count]->is_directory = entry->d_type == DT_DIR;
            file_count++;
        }
    }

    closedir(dir);
    *count = file_count;
    return files;
}

char* read_file(const char* path, size_t* size) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(file_size + 1);
    fread(data, 1, file_size, file);
    fclose(file);

    *size = file_size;
    data[file_size] = '\0';
    return data;
}

bool write_file(const char* path, const char* data, size_t size) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return false;
    }

    fwrite(data, 1, size, file);
    fclose(file);
    return true;
}

#endif