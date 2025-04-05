#include "cnt_semaphore.h"

// Semafor Oluşturma Fonksiyonu
SemaphoreHandle Semaphore_Create(uint32_t initialCount) {
    // OSAL semafor oluşturma fonksiyonunu çağır
    return OSAL_SemaphoreCreate(initialCount);
}

// Semafor Alma Fonksiyonu
bool Semaphore_Take(SemaphoreHandle semaphore, uint32_t timeout) {
    // OSAL semafor alma fonksiyonunu çağır
    return OSAL_SemaphoreTake(semaphore, timeout);
}

// Semafor Verme Fonksiyonu
bool Semaphore_Give(SemaphoreHandle semaphore) {
    // OSAL semafor verme fonksiyonunu çağır
    return OSAL_SemaphoreGive(semaphore);
}