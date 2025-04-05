#ifndef CNT_CORE_H
#define CNT_CORE_H

#include <stddef.h> // size_t için
#include <stdint.h> // uint32_t gibi tamsayı türleri için

// Programdan Çıkış Fonksiyonu
void cnt_exit(int status);

// Bellek Ayırma Fonksiyonu
void* cnt_malloc(size_t size);

// Bellek Serbest Bırakma Fonksiyonu
void cnt_free(void* ptr);

// Tamsayı Yazdırma Fonksiyonu
void cnt_print_int(int value);

// Kayan Noktalı Sayı Yazdırma Fonksiyonu
void cnt_print_float(float value);

// String Yazdırma Fonksiyonu
void cnt_print_string(char* str);

#endif