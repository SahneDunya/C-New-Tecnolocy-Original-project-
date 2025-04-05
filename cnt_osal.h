#ifndef CNT_OSAL_H
#define CNT_OSAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// İş Parçacığı Tanımlayıcısı
typedef void* OSAL_ThreadHandle;

// Karşılıklı Dışlama (Mutex) Tanımlayıcısı
typedef void* OSAL_MutexHandle;

// Semaför Tanımlayıcısı
typedef void* OSAL_SemaphoreHandle;

// Kuyruk Tanımlayıcısı
typedef void* OSAL_QueueHandle;

// İş Parçacığı Oluşturma Fonksiyonu
OSAL_ThreadHandle OSAL_ThreadCreate(void (*entry)(void*), void* argument, uint32_t stackSize, uint32_t priority);

// İş Parçacığı Geciktirme Fonksiyonu
void OSAL_ThreadDelay(uint32_t milliseconds);

// Karşılıklı Dışlama (Mutex) Oluşturma Fonksiyonu
OSAL_MutexHandle OSAL_MutexCreate(void);

// Karşılıklı Dışlama (Mutex) Kilitleme Fonksiyonu
bool OSAL_MutexLock(OSAL_MutexHandle mutex);

// Karşılıklı Dışlama (Mutex) Kilidini Açma Fonksiyonu
bool OSAL_MutexUnlock(OSAL_MutexHandle mutex);

// Semaför Oluşturma Fonksiyonu
OSAL_SemaphoreHandle OSAL_SemaphoreCreate(uint32_t initialCount);

// Semaför Alma Fonksiyonu
bool OSAL_SemaphoreTake(OSAL_SemaphoreHandle semaphore, uint32_t timeout);

// Semaför Verme Fonksiyonu
bool OSAL_SemaphoreGive(OSAL_SemaphoreHandle semaphore);

// Kuyruk Oluşturma Fonksiyonu
OSAL_QueueHandle OSAL_QueueCreate(uint32_t messageSize, uint32_t queueLength);

// Kuyruğa Mesaj Gönderme Fonksiyonu
bool OSAL_QueueSend(OSAL_QueueHandle queue, void* message, uint32_t timeout);

// Kuyruktan Mesaj Alma Fonksiyonu
bool OSAL_QueueReceive(OSAL_QueueHandle queue, void* message, uint32_t timeout);

#endif