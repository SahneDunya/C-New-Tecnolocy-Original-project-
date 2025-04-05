#ifndef CNT_THREAD_H
#define CNT_THREAD_H

#include <stdbool.h>
#include <stddef.h>

// İş Parçacığı Yapısı
typedef struct {
    // Platforma özgü iş parçacığı tanıtıcısı
    #ifdef _WIN32
        HANDLE thread_handle;
        DWORD thread_id;
    #else
        pthread_t thread_id;
    #endif
} Thread;

// İş Parçacığı Fonksiyonu İşaretçisi
typedef void* (*ThreadFunction)(void* arg);

// İş Parçacığı Oluşturma Fonksiyonu
Thread* create_thread(ThreadFunction function, void* arg);

// İş Parçacığı Bekleme Fonksiyonu
int wait_for_thread(Thread* thread);

// İş Parçacığı Sonlandırma Fonksiyonu
bool terminate_thread(Thread* thread);

// İş Parçacığı Serbest Bırakma Fonksiyonu
void free_thread(Thread* thread);

#endif