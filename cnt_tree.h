#ifndef CNT_TREE_H
#define CNT_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Ağaç Düğümü Yapısı
typedef struct TreeNode {
    void* data;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// Ağaç Yapısı
typedef struct {
    TreeNode* root;
    uint32_t size;
} Tree;

// Ağaç Başlatma Fonksiyonu
void Tree_Init(Tree* tree);

// Ağaca Düğüm Ekleme Fonksiyonu
bool Tree_Insert(Tree* tree, void* data);

// Ağaçta Düğüm Arama Fonksiyonu
TreeNode* Tree_Search(Tree* tree, void* data);

// Ağaçtan Düğüm Silme Fonksiyonu
bool Tree_Delete(Tree* tree, void* data);

// Ağaç Boyutunu Alma Fonksiyonu
uint32_t Tree_Size(Tree* tree);

// Ağaç Boş Mu Kontrol Fonksiyonu
bool Tree_IsEmpty(Tree* tree);

// Ağaç Temizleme Fonksiyonu
void Tree_Clear(Tree* tree);

#endif