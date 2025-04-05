#include <stdio.h>
#include <stdlib.h>
#include "cnt_stack.h"

Stack* create_stack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

void stack_push(Stack* stack, void* data) {
    StackNode* new_node = (StackNode*)malloc(sizeof(StackNode));
    new_node->data = data;
    new_node->next = stack->top;
    stack->top = new_node;
    stack->size++;
}

void* stack_pop(Stack* stack) {
    if (stack->top == NULL) {
        return NULL;
    }

    StackNode* removed_node = stack->top;
    void* removed_data = removed_node->data;
    stack->top = removed_node->next;
    free(removed_node);
    stack->size--;
    return removed_data;
}

void* stack_peek(Stack* stack) {
    if (stack->top == NULL) {
        return NULL;
    }

    return stack->top->data;
}

size_t stack_size(Stack* stack) {
    return stack->size;
}

bool stack_is_empty(Stack* stack) {
    return stack->size == 0;
}

void stack_clear(Stack* stack) {
    while (stack->top != NULL) {
        StackNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->size = 0;
}

void free_stack(Stack* stack) {
    stack_clear(stack);
    free(stack);
}