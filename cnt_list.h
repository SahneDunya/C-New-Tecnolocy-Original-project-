#ifndef CNT_LIST_H
#define CNT_LIST_H

#include <stdbool.h>

// Liste Öğesi Yapısı
typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

// Liste Yapısı
typedef struct {
    ListNode* head;
    ListNode* tail;
    int size;
} List;

// Liste Oluşturma Fonksiyonu
List* create_list();

// Liste Öğesi Ekleme Fonksiyonları
void list_append(List* list, void* data);
void list_prepend(List* list, void* data);
void list_insert(List* list, void* data, int index);

// Liste Öğesi Kaldırma Fonksiyonları
void* list_remove(List* list, int index);
void list_remove_first(List* list);
void list_remove_last(List* list);

// Liste Öğesine Erişim Fonksiyonu
void* list_get(List* list, int index);

// Liste Boyutu Fonksiyonu
int list_size(List* list);

// Liste Boş Mu Fonksiyonu
bool list_is_empty(List* list);

// Liste Temizleme Fonksiyonu
void list_clear(List* list);

// Liste Serbest Bırakma Fonksiyonu
void free_list(List* list);

#endif