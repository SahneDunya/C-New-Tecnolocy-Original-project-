#ifndef CNT_STACK_H
#define CNT_STACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Yığın Yapısı
typedef struct {
    void** items;
    uint32_t capacity;
    uint32_t top;
} Stack;

// Yığın Başlatma Fonksiyonu
bool Stack_Init(Stack* stack, uint32_t capacity);

// Yığına Öğe Ekleme Fonksiyonu
bool Stack_Push(Stack* stack, void* item);

// Yığından Öğe Çıkarma Fonksiyonu
void* Stack_Pop(Stack* stack);

// Yığının En Üstündeki Öğeyi Alma Fonksiyonu
void* Stack_Peek(Stack* stack);

// Yığının Boş Olup Olmadığını Kontrol Etme Fonksiyonu
bool Stack_IsEmpty(Stack* stack);

// Yığının Dolu Olup Olmadığını Kontrol Etme Fonksiyonu
bool Stack_IsFull(Stack* stack);

// Yığını Temizleme Fonksiyonu
void Stack_Clear(Stack* stack);

// Yığını Yok Etme Fonksiyonu
void Stack_Destroy(Stack* stack);

#endif