#include "cnt_thread.h"

// İş Parçacığı Oluşturma Fonksiyonu
ThreadHandle Thread_Create(void (*entry)(void*), void* argument, uint32_t stackSize, uint32_t priority) {
    // OSAL iş parçacığı oluşturma fonksiyonunu çağır
    return OSAL_ThreadCreate(entry, argument, stackSize, priority);
}

// İş Parçacığı Geciktirme Fonksiyonu
void Thread_Delay(uint32_t milliseconds) {
    // OSAL iş parçacığı geciktirme fonksiyonunu çağır
    OSAL_ThreadDelay(milliseconds);
}