#ifndef CNT_STACK_H
#define CNT_STACK_H

#include <stdbool.h>
#include <stddef.h>

// Yığın Öğesi Yapısı
typedef struct StackNode {
    void* data;
    struct StackNode* next;
} StackNode;

// Yığın Yapısı
typedef struct {
    StackNode* top;
    size_t size;
} Stack;

// Yığın Oluşturma Fonksiyonu
Stack* create_stack();

// Yığın Öğesi Ekleme Fonksiyonu
void stack_push(Stack* stack, void* data);

// Yığın Öğesi Kaldırma Fonksiyonu
void* stack_pop(Stack* stack);

// Yığın Öğesine Erişim Fonksiyonu
void* stack_peek(Stack* stack);

// Yığın Boyutu Fonksiyonu
size_t stack_size(Stack* stack);

// Yığın Boş Mu Fonksiyonu
bool stack_is_empty(Stack* stack);

// Yığın Temizleme Fonksiyonu
void stack_clear(Stack* stack);

// Yığın Serbest Bırakma Fonksiyonu
void free_stack(Stack* stack);

#endif