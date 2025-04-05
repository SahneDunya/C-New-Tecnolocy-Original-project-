#ifndef CNT_QUEUE_H
#define CNT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

// Kuyruk Öğesi Yapısı
typedef struct QueueNode {
    void* data;
    struct QueueNode* next;
} QueueNode;

// Kuyruk Yapısı
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    size_t size;
} Queue;

// Kuyruk Oluşturma Fonksiyonu
Queue* create_queue();

// Kuyruk Öğesi Ekleme Fonksiyonu
void queue_enqueue(Queue* queue, void* data);

// Kuyruk Öğesi Kaldırma Fonksiyonu
void* queue_dequeue(Queue* queue);

// Kuyruk Öğesine Erişim Fonksiyonu
void* queue_peek(Queue* queue);

// Kuyruk Boyutu Fonksiyonu
size_t queue_size(Queue* queue);

// Kuyruk Boş Mu Fonksiyonu
bool queue_is_empty(Queue* queue);

// Kuyruk Temizleme Fonksiyonu
void queue_clear(Queue* queue);

// Kuyruk Serbest Bırakma Fonksiyonu
void free_queue(Queue* queue);

#endif