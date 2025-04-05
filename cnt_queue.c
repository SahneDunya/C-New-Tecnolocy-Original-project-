#include <stdio.h>
#include <stdlib.h>
#include "cnt_queue.h"

Queue* create_queue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

void queue_enqueue(Queue* queue, void* data) {
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->data = data;
    new_node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

    queue->size++;
}

void* queue_dequeue(Queue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    QueueNode* removed_node = queue->front;
    void* removed_data = removed_node->data;
    queue->front = removed_node->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(removed_node);
    queue->size--;
    return removed_data;
}

void* queue_peek(Queue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    return queue->front->data;
}

size_t queue_size(Queue* queue) {
    return queue->size;
}

bool queue_is_empty(Queue* queue) {
    return queue->size == 0;
}

void queue_clear(Queue* queue) {
    while (queue->front != NULL) {
        QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }

    queue->rear = NULL;
    queue->size = 0;
}

void free_queue(Queue* queue) {
    queue_clear(queue);
    free(queue);
}