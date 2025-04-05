#include <stdio.h>
#include <stdlib.h>
#include "cnt_list.h"

List* create_list() {
    List* list = (List*)malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void list_append(List* list, void* data) {
    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    new_node->data = data;
    new_node->next = NULL;

    if (list->tail == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->size++;
}

void list_prepend(List* list, void* data) {
    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    new_node->data = data;
    new_node->next = list->head;

    list->head = new_node;
    if (list->tail == NULL) {
        list->tail = new_node;
    }

    list->size++;
}

void list_insert(List* list, void* data, int index) {
    if (index < 0 || index > list->size) {
        fprintf(stderr, "Hata: Geçersiz indeks.\n");
        return;
    }

    if (index == 0) {
        list_prepend(list, data);
        return;
    }

    if (index == list->size) {
        list_append(list, data);
        return;
    }

    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    new_node->data = data;

    ListNode* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;

    list->size++;
}

void* list_remove(List* list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Hata: Geçersiz indeks.\n");
        return NULL;
    }

    if (index == 0) {
        return list_remove_first(list);
    }

    if (index == list->size - 1) {
        return list_remove_last(list);
    }

    ListNode* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    ListNode* removed_node = current->next;
    current->next = removed_node->next;

    void* removed_data = removed_node->data;
    free(removed_node);

    list->size--;
    return removed_data;
}

void list_remove_first(List* list) {
    if (list->head == NULL) {
        return NULL;
    }

    ListNode* removed_node = list->head;
    list->head = removed_node->next;

    if (list->head == NULL) {
        list->tail = NULL;
    }

    void* removed_data = removed_node->data;
    free(removed_node);

    list->size--;
    return removed_data;
}

void list_remove_last(List* list) {
    if (list->tail == NULL) {
        return NULL;
    }

    if (list->head == list->tail) {
        return list_remove_first(list);
    }

    ListNode* current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }

    ListNode* removed_node = list->tail;
    current->next = NULL;
    list->tail = current;

    void* removed_data = removed_node->data;
    free(removed_node);

    list->size--;
    return removed_data;
}

void* list_get(List* list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Hata: Geçersiz indeks.\n");
        return NULL;
    }

    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }

    return current->data;
}

int list_size(List* list) {
    return list->size;
}

bool list_is_empty(List* list) {
    return list->size == 0;
}

void list_clear(List* list) {
    ListNode* current = list->head;
    while (current != NULL) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void free_list(List* list) {
    list_clear(list);
    free(list);
}