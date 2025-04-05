#include "library_selector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kütüphane Seçimi Yapılandırma Yapısı
typedef struct {
    int selected_library;
} LibrarySelectionConfig;

// Kütüphane Seçimi Fonksiyonu
void select_library(LibrarySelectionConfig* config, int library_type) {
    if (library_type == USE_STANDARD_LIBRARY || library_type == USE_EMBEDDED_LIBRARY || library_type == USE_NO_LIBRARY) {
        config->selected_library = library_type;
    } else {
        config->selected_library = USE_STANDARD_LIBRARY; // Varsayılan olarak standart kütüphaneyi seç
        fprintf(stderr, "Geçersiz kütüphane seçimi. Standart kütüphane kullanılıyor.\n");
    }
}

// Kütüphane Seçimini Kontrol Etme Fonksiyonu
int get_selected_library(LibrarySelectionConfig* config) {
    return config->selected_library;
}