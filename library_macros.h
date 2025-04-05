#ifndef LIBRARY_MACROS_H
#define LIBRARY_MACROS_H

// Kütüphane Seçimi Makroları
#define USE_STANDARD_LIBRARY 1
#define USE_EMBEDDED_LIBRARY 2
#define USE_NO_LIBRARY 0

// Kütüphane Seçimi Direktifleri
#define NOT_INCLUDE_STANDARD_LIBRARY "#not include <standard c library>"
#define NOT_INCLUDE_EMBEDDED_LIBRARY "#not include <embedded c library>"

// Kütüphane Seçimi Hata Kodları
#define LIBRARY_SELECTION_SUCCESS 0
#define LIBRARY_SELECTION_ERROR -1

// Kütüphane Seçimi Hata Mesajları
#define LIBRARY_SELECTION_ERROR_MESSAGE "Kütüphane seçimi hatası."

// Diğer Kütüphane Makroları (Örnek)
#define CNT_LIBRARY_VERSION "1.0.0"

#endif