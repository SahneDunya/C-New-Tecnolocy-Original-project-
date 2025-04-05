#include "cnt_builtin_functions.h"
#include <stdio.h>
#include <string.h>

// Yerleşik fonksiyonların implementasyonları

void builtin_print(void* argument) {
    // Argument'ın CNT dilindeki string tipinde olduğunu varsayıyoruz.
    // Gerçek implementasyonda tip kontrolü ve uygun yazdırma mekanizması gerekebilir.
    if (argument) {
        printf("%s\n", (char*)argument);
    } else {
        printf("null\n");
    }
}

// ... diğer yerleşik fonksiyonların implementasyonları buraya eklenebilir ...