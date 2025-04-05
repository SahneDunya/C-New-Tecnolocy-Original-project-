#ifndef CNT_QUEUE_H
#define CNT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_osal.h"

// Kuyruk Tanımlayıcısı
typedef OSAL_QueueHandle QueueHandle;

// Kuyruk Oluşturma Fonksiyonu
QueueHandle Queue_Create(uint32_t messageSize, uint32_t queueLength);

// Kuyruğa Mesaj Gönderme Fonksiyonu
bool Queue_Send(QueueHandle queue, void* message, uint32_t timeout);

// Kuyruktan Mesaj Alma Fonksiyonu
bool Queue_Receive(QueueHandle queue, void* message, uint32_t timeout);

#endif