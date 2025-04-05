#include "cnt_compiletime.h"
#include <stdio.h>

// Derleme Zamanı Hata Raporlama Fonksiyonu
void compile_error(CompileError error) {
    printf("Derleme hatası: %s\n", error.message);
    // Derleme işlemini durdur
    exit(1);
}

// Derleme Zamanı Sahiplik Kontrol Fonksiyonu
bool compile_check_ownership(void* owner) {
    // Sahiplik kurallarını kontrol et
    // ... sahiplik kontrol mantığı ...

    // Basit bir örnek: sahiplik devredilmişse hata ver
    if (owner == NULL) {
        CompileError error = {COMPILE_ERROR_OWNERSHIP, "Sahiplik devredilmiş"};
        compile_error(error);
        return false;
    }
    return true;
}

// Derleme Zamanı Ödünç Alma Kontrol Fonksiyonu
bool compile_check_borrow(void* borrow) {
    // Ödünç alma kurallarını kontrol et
    // ... ödünç alma kontrol mantığı ...

    // Basit bir örnek: aynı anda birden fazla değiştirilebilir ödünç alma varsa hata ver
    if (borrow == NULL) {
        CompileError error = {COMPILE_ERROR_BORROW, "Aynı anda birden fazla değiştirilebilir ödünç alma"};
        compile_error(error);
        return false;
    }
    return true;
}

// Derleme Zamanı Yaşam Süresi Kontrol Fonksiyonu
bool compile_check_lifetime(void* lifetime) {
    // Yaşam süresi kurallarını kontrol et
    // ... yaşam süresi kontrol mantığı ...

    // Basit bir örnek: ödünç alınan referansın yaşam süresi sahibinden uzunsa hata ver
    if (lifetime == NULL) {
        CompileError error = {COMPILE_ERROR_LIFETIME, "Ödünç alınan referansın yaşam süresi sahibi"} 
    }
}