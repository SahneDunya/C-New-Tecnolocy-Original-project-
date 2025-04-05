#ifndef CNT_LIST_H
#define CNT_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Liste Düğümü Yapısı
typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

// Liste Yapısı
typedef struct {
    ListNode* head;
    ListNode* tail;
    uint32_t size;
} List;

// Liste Başlatma Fonksiyonu
void List_Init(List* list);

// Liste Başına Düğüm Ekleme Fonksiyonu
bool List_PushFront(List* list, void* data);

// Liste Sonuna Düğüm Ekleme Fonksiyonu
bool List_PushBack(List* list, void* data);

// Liste Başına Düğüm Çıkarma Fonksiyonu
void* List_PopFront(List* list);

// Liste Sonundan Düğüm Çıkarma Fonksiyonu
void* List_PopBack(List* list);

// Liste Düğümü Ekleme Fonksiyonu (Belirli Konuma)
bool List_Insert(List* list, void* data, uint32_t index);

// Liste Düğümü Çıkarma Fonksiyonu (Belirli Konumdan)
void* List_Remove(List* list, uint32_t index);

// Liste Boyutunu Alma Fonksiyonu
uint32_t List_Size(List* list);

// Liste Boş Mu Kontrol Fonksiyonu
bool List_IsEmpty(List* list);

// Liste Temizleme Fonksiyonu
void List_Clear(List* list);

#endif