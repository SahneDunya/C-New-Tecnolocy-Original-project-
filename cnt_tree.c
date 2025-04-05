#include "cnt_tree.h"
#include <stdlib.h>

// Ağaç Başlatma Fonksiyonu
void Tree_Init(Tree* tree) {
    if (tree != NULL) {
        tree->root = NULL;
        tree->size = 0;
    }
}

// Ağaca Düğüm Ekleme Fonksiyonu
bool Tree_Insert(Tree* tree, void* data) {
    if (tree == NULL) {
        return false;
    }

    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        return false;
    }

    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;

    if (tree->root == NULL) {
        tree->root = newNode;
    } else {
        // İkili ağaç ekleme mantığı (basit örnek)
        TreeNode* current = tree->root;
        while (1) {
            if (data < current->data) { // Veri karşılaştırma (örnek)
                if (current->left == NULL) {
                    current->left = newNode;
                    break;
                }
                current = current->left;
            } else {
                if (current->right == NULL) {
                    current->right = newNode;
                    break;
                }
                current = current->right;
            }
        }
    }

    tree->size++;
    return true;
}

// Ağaçta Düğüm Arama Fonksiyonu
TreeNode* Tree_Search(Tree* tree, void* data) {
    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }

    TreeNode* current = tree->root;
    while (current != NULL) {
        if (data == current->data) { // Veri karşılaştırma (örnek)
            return current;
        } else if (data < current->data) { // Veri karşılaştırma (örnek)
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

// Ağaçtan Düğüm Silme Fonksiyonu
bool Tree_Delete(Tree* tree, void* data) {
    // Ağaçtan düğüm silme mantığı (basit örnek)
    // ...
    return false;
}

// Ağaç Boyutunu Alma Fonksiyonu
uint32_t Tree_Size(Tree* tree) {
    if (tree == NULL) {
        return 0;
    }
    return tree->size;
}

// Ağaç Boş Mu Kontrol Fonksiyonu
bool Tree_IsEmpty(Tree* tree) {
    if (tree == NULL) {
        return true;
    }
    return tree->size == 0;
}

// Ağaç Temizleme Fonksiyonu
void Tree_Clear(Tree* tree) {
    // Ağaç temizleme mantığı (basit örnek)
    // ...
}