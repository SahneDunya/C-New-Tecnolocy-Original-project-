#include "cnt_interface.h"
#include <stdlib.h>

// Arayüz Uygulaması Oluşturma Fonksiyonu
InterfaceImplementation* interface_create_implementation(const Interface* interface, void* object) {
    InterfaceImplementation* impl = (InterfaceImplementation*)malloc(sizeof(InterfaceImplementation));
    if (impl == NULL) {
        return NULL; // Bellek ayırma hatası
    }
    impl->interface = interface;
    impl->object = object;
    return impl;
}

// Arayüz Uygulamasını Serbest Bırakma Fonksiyonu
void interface_free_implementation(InterfaceImplementation* impl) {
    free(impl);
}

// Arayüz Fonksiyonlarını Çağırma Fonksiyonları (örnek)
void interface_call_function1(InterfaceImplementation* impl, int arg) {
    if (impl->interface->function1 != NULL) {
        impl->interface->function1(impl->object, arg);
    }
}

int interface_call_function2(InterfaceImplementation* impl, const char* arg) {
    if (impl->interface->function2 != NULL) {
        return impl->interface->function2(impl->object, arg);
    }
    return 0; // Varsayılan değer
}

// ... diğer arayüz fonksiyonlarını çağırma fonksiyonları ...