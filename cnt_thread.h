#ifndef CNT_THREAD_H
#define CNT_THREAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cnt_osal.h"

// İş Parçacığı Tanımlayıcısı
typedef OSAL_ThreadHandle ThreadHandle;

// İş Parçacığı Oluşturma Fonksiyonu
ThreadHandle Thread_Create(void (*entry)(void*), void* argument, uint32_t stackSize, uint32_t priority);

// İş Parçacığı Geciktirme Fonksiyonu
void Thread_Delay(uint32_t milliseconds);

#endif