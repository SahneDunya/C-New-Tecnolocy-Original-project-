#ifndef CNT_STDIO_H
#define CNT_STDIO_H

#include <stdio.h> // Gerekli C standart kütüphane tanımları için
#include <stdbool.h>

// Standart çıktı fonksiyonları

// Tek bir string veya temel veri tipini çıktıya yazar (otomatik olarak yeni satır ekler)
void cnt_println(const char* format, ...);

// Biçimlendirilmiş çıktıyı standart çıktıya yazar (C'deki printf benzeri)
void cnt_printf(const char* format, ...);

// Standart girdi fonksiyonları

// Standart girdiden bir satır okur (belleği dinamik olarak ayırır, çağıran tarafın serbest bırakması gerekir)
char* cnt_read_line();

// Standart girdiden formatlı veri okur (C'deki scanf benzeri - dikkatli kullanılmalı)
int cnt_scanf(const char* format, ...);

// Standart hata çıktısı fonksiyonu
void cnt_eprintln(const char* format, ...);

#endif