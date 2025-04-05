#ifndef CNT_PROCESS_H
#define CNT_PROCESS_H

#include <stdbool.h>
#include <stddef.h>

// İşlem Yapısı
typedef struct {
    int pid;
    // ... diğer işlem bilgileri ...
} Process;

// İşlem Oluşturma Fonksiyonu
Process* create_process(const char* command, char** args);

// İşlem Bekleme Fonksiyonu
int wait_for_process(Process* process);

// İşlem Sonlandırma Fonksiyonu
bool terminate_process(Process* process);

// İşlem Bilgisi Alma Fonksiyonu
Process* get_process_info(int pid);

// İşlem Serbest Bırakma Fonksiyonu
void free_process(Process* process);

#endif