#ifndef LIBRARY_ERRORS_H
#define LIBRARY_ERRORS_H

// Kütüphane Seçimi Hata Kodları
#define LIBRARY_SELECTION_SUCCESS 0
#define LIBRARY_SELECTION_ERROR -1
#define LIBRARY_NOT_FOUND_ERROR -2
#define LIBRARY_CONFIG_ERROR -3
#define LIBRARY_DEPENDENCY_ERROR -4

// Kütüphane Seçimi Hata Mesajları
#define LIBRARY_SELECTION_ERROR_MESSAGE "Kütüphane seçimi hatası: %s"
#define LIBRARY_NOT_FOUND_ERROR_MESSAGE "Kütüphane bulunamadı: %s"
#define LIBRARY_CONFIG_ERROR_MESSAGE "Kütüphane yapılandırma hatası: %s"
#define LIBRARY_DEPENDENCY_ERROR_MESSAGE "Kütüphane bağımlılık hatası: %s"

#endif