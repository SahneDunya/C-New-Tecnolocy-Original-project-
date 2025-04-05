#ifndef CNT_MUTEX_H
#define CNT_MUTEX_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_osal.h"

// Mutex Tanımlayıcısı
typedef OSAL_MutexHandle MutexHandle;

// Mutex Oluşturma Fonksiyonu
MutexHandle Mutex_Create(void);

// Mutex Kilitleme Fonksiyonu
bool Mutex_Lock(MutexHandle mutex);

// Mutex Kilidini Açma Fonksiyonu
bool Mutex_Unlock(MutexHandle mutex);

#endif