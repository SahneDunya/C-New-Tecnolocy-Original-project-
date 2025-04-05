#ifndef CNT_SEMAPHORE_H
#define CNT_SEMAPHORE_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_osal.h"

// Semafor Tanımlayıcısı
typedef OSAL_SemaphoreHandle SemaphoreHandle;

// Semafor Oluşturma Fonksiyonu
SemaphoreHandle Semaphore_Create(uint32_t initialCount);

// Semafor Alma Fonksiyonu
bool Semaphore_Take(SemaphoreHandle semaphore, uint32_t timeout);

// Semafor Verme Fonksiyonu
bool Semaphore_Give(SemaphoreHandle semaphore);

#endif