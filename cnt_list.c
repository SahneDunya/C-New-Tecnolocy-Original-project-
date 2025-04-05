#include "cnt_list.h"
#include <stdlib.h>

// Liste Başlatma Fonksiyonu
void List_Init(List* list) {
    if (list != NULL) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
}

// Liste Başına Düğüm Ekleme Fonksiyonu
bool List_PushFront(List* list, void* data) {
    if (list == NULL) {
        return false;
    }

    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (newNode == NULL) {
        return false;
    }

    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;

    if (list->tail == NULL) {
        list->tail = newNode;
    }

    list->size++;
    return true;
}

// Liste Sonuna Düğüm Ekleme Fonksiyonu
bool List_PushBack(List* list, void* data) {
    if (list == NULL) {
        return false;
    }

    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (newNode == NULL) {
        return false;
    }

    newNode->data = data;
    newNode->next = NULL;

    if (list->tail == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->size++;
    return true;
}

// Liste Başına Düğüm Çıkarma Fonksiyonu
void* List_PopFront(List* list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    ListNode* temp = list->head;
    void* data = temp->data;

    list->head = list->head->next;
    free(temp);

    if (list->head == NULL) {
        list->tail = NULL;
    }

    list->size--;
    return data;
}

// Liste Sonundan Düğüm Çıkarma Fonksiyonu
void* List_PopBack(List* list) {
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }

    ListNode* temp = list->tail;
    void* data = temp->data;

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        ListNode* current = list->head;
        while (current->next != list->tail) {
            current = current->next;
        }
        current->next = NULL;
        list->tail = current;
    }

    free(temp);
    list->size--;
    return data;
}

// Liste Düğümü Ekleme Fonksiyonu (Belirli Konuma)
bool List_Insert(List* list, void* data, uint32_t index) {
    if (list == NULL || index > list->size) {
        return false;
    }

    if (index == 0) {
        return List_PushFront(list, data);
    }

    if (index == list->size) {
        return List_PushBack(list, data);
    }

    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (newNode == NULL) {
        return false;
    }

    newNode->data = data;

    ListNode* current = list->head;
    for (uint32_t i = 0; i < index - 1; i++) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;

    list->size++;
    return true;
}

// Liste Düğümü Çıkarma Fonksiyonu (Belirli Konumdan)
void* List_Remove(List* list, uint32_t index) {
    if (list == NULL || index >= list->size) {
        return NULL;
    }

    if (index == 0) {
        return List_PopFront(list);
    }

    if (index == list->size - 1) {
        return List_PopBack(list);
    }

    ListNode* current = list->head;
    for (uint32_t i = 0; i < index - 1; i++) {
        current = current->next;
    }

    ListNode* temp = current->next;
    void* data = temp->data;

    current->next = temp->next;
    free(temp);

    list->size--;
    return data;
}

// Liste Boyutunu Alma Fonksiyonu
uint32_t List_Size(List* list) {
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

// Liste Boş Mu Kontrol Fonksiyonu
bool List_IsEmpty(List* list) {
    if (list == NULL) {
        return true;
    }
    return list->size == 0;
}

// Liste Temizleme Fonksiyonu
void List_Clear(List* list) {
    if (list == NULL) {
        return;
    }

    ListNode* current = list->head;
    while (current != NULL) {
        ListNode* temp = current;
        current = current->next;
        free(temp);
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}