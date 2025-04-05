#ifndef LIBRARY_SELECTOR_H
#define LIBRARY_SELECTOR_H

#include <stdbool.h>
#include <stddef.h>

// Kütüphane Seçimi Makroları
#define USE_STANDARD_LIBRARY 1
#define USE_EMBEDDED_LIBRARY 2
#define USE_NO_LIBRARY 0

// Kütüphane Seçimi Yapılandırma Yapısı
typedef struct {
    int selected_library;
} LibrarySelectionConfig;

// Kütüphane Seçimi Fonksiyonu
void select_library(LibrarySelectionConfig* config, int library_type);

// Kütüphane Seçimini Kontrol Etme Fonksiyonu
int get_selected_library(LibrarySelectionConfig* config);

// Kütüphane Seçimi Hata Kodları
#define LIBRARY_SELECTION_SUCCESS 0
#define LIBRARY_SELECTION_ERROR -1

#endif