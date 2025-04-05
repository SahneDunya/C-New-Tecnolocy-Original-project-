#include "cnt_mutex.h"

// Mutex Oluşturma Fonksiyonu
MutexHandle Mutex_Create(void) {
    // OSAL mutex oluşturma fonksiyonunu çağır
    return OSAL_MutexCreate();
}

// Mutex Kilitleme Fonksiyonu
bool Mutex_Lock(MutexHandle mutex) {
    // OSAL mutex kilitleme fonksiyonunu çağır
    return OSAL_MutexLock(mutex);
}

// Mutex Kilidini Açma Fonksiyonu
bool Mutex_Unlock(MutexHandle mutex) {
    // OSAL mutex kilidini açma fonksiyonunu çağır
    return OSAL_MutexUnlock(mutex);
}