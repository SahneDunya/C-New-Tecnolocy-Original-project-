#ifndef CNT_IO_H
#define CNT_IO_H

#include <stdio.h>
#include <stdlib.h>

// Tamsayı okuma fonksiyonu
int cnt_read_int();

// Kayan noktalı sayı okuma fonksiyonu
float cnt_read_float();

// String okuma fonksiyonu
char* cnt_read_string();

// Tamsayı yazdırma fonksiyonu
void cnt_print_int(int value);

// Kayan noktalı sayı yazdırma fonksiyonu
void cnt_print_float(float value);

// String yazdırma fonksiyonu
void cnt_print_string(char* str);

// Karakter yazdırma fonksiyonu
void cnt_print_char(char c);

// Yeni satır yazdırma fonksiyonu
void cnt_print_newline();

#endif