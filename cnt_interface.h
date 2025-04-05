#ifndef CNT_INTERFACE_H
#define CNT_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>

// Arayüz Yapısı
typedef struct Interface {
    const char* name;
    // Arayüz fonksiyon işaretçileri (örnek)
    void (*function1)(void* obj, int arg);
    int (*function2)(void* obj, const char* arg);
    // ... diğer arayüz fonksiyon işaretçileri ...
} Interface;

// Arayüz Uygulama Yapısı
typedef struct InterfaceImplementation {
    const Interface* interface;
    void* object; // Arayüzü uygulayan nesne
} InterfaceImplementation;

// Arayüz Uygulaması Oluşturma Fonksiyonu
InterfaceImplementation* interface_create_implementation(const Interface* interface, void* object);

// Arayüz Uygulamasını Serbest Bırakma Fonksiyonu
void interface_free_implementation(InterfaceImplementation* impl);

// Arayüz Fonksiyonlarını Çağırma Fonksiyonları (örnek)
void interface_call_function1(InterfaceImplementation* impl, int arg);
int interface_call_function2(InterfaceImplementation* impl, const char* arg);
// ... diğer arayüz fonksiyonlarını çağırma fonksiyonları ...

#endif