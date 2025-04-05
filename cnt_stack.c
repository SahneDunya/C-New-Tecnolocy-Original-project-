#include "cnt_stack.h"
#include <stdlib.h>

// Yığın Başlatma Fonksiyonu
bool Stack_Init(Stack* stack, uint32_t capacity) {
    if (stack == NULL || capacity == 0) {
        return false;
    }

    stack->items = (void**)malloc(capacity * sizeof(void*));
    if (stack->items == NULL) {
        return false;
    }

    stack->capacity = capacity;
    stack->top = 0;

    return true;
}

// Yığına Öğe Ekleme Fonksiyonu
bool Stack_Push(Stack* stack, void* item) {
    if (stack == NULL || stack->top >= stack->capacity) {
        return false;
    }

    stack->items[stack->top++] = item;
    return true;
}

// Yığından Öğe Çıkarma Fonksiyonu
void* Stack_Pop(Stack* stack) {
    if (stack == NULL || stack->top == 0) {
        return NULL;
    }

    return stack->items[--stack->top];
}

// Yığının En Üstündeki Öğeyi Alma Fonksiyonu
void* Stack_Peek(Stack* stack) {
    if (stack == NULL || stack->top == 0) {
        return NULL;
    }

    return stack->items[stack->top - 1];
}

// Yığının Boş Olup Olmadığını Kontrol Etme Fonksiyonu
bool Stack_IsEmpty(Stack* stack) {
    if (stack == NULL) {
        return true;
    }

    return stack->top == 0;
}

// Yığının Dolu Olup Olmadığını Kontrol Etme Fonksiyonu
bool Stack_IsFull(Stack* stack) {
    if (stack == NULL) {
        return false;
    }

    return stack->top == stack->capacity;
}

// Yığını Temizleme Fonksiyonu
void Stack_Clear(Stack* stack) {
    if (stack != NULL) {
        stack->top = 0;
    }
}

// Yığını Yok Etme Fonksiyonu
void Stack_Destroy(Stack* stack) {
    if (stack != NULL) {
        free(stack->items);
        stack->items = NULL;
        stack->capacity = 0;
        stack->top = 0;
    }
}