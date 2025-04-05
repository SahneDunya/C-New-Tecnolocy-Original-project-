#include "cnt_queue.h"

// Kuyruk Oluşturma Fonksiyonu
QueueHandle Queue_Create(uint32_t messageSize, uint32_t queueLength) {
    // OSAL kuyruk oluşturma fonksiyonunu çağır
    return OSAL_QueueCreate(messageSize, queueLength);
}

// Kuyruğa Mesaj Gönderme Fonksiyonu
bool Queue_Send(QueueHandle queue, void* message, uint32_t timeout) {
    // OSAL kuyruğa mesaj gönderme fonksiyonunu çağır
    return OSAL_QueueSend(queue, message, timeout);
}

// Kuyruktan Mesaj Alma Fonksiyonu
bool Queue_Receive(QueueHandle queue, void* message, uint32_t timeout) {
    // OSAL kuyruktan mesaj alma fonksiyonunu çağır
    return OSAL_QueueReceive(queue, message, timeout);
}