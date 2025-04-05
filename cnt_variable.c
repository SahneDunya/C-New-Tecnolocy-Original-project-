#include "cnt_variable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Variable* variable_create(const char* name, Type* type, bool mutable) {
    Variable* var = (Variable*)malloc(sizeof(Variable));
    if (var == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    var->name = strdup(name);
    var->type = type;
    var->value = NULL; // Başlangıçta değer atanmamış olabilir
    var->is_mutable = mutable;
    // Modern özellikler için varsayılan değerler atanabilir
    var->is_owned = true; // Varsayılan olarak sahibi olabilir
    var->owner = NULL;
    var->lifetime_counter = 0;
    return var;
}

bool variable_set_value(Variable* var, const void* new_value) {
    if (!var->is_mutable && var->value != NULL) {
        fprintf(stderr, "Hata: Değiştirilemeyen değişkene değer atanmaya çalışılıyor.\n");
        return false;
    }
    // Dikkat: Burada sadece pointer'ı kopyalıyoruz. Değerin kendisini kopyalamak gerekebilir.
    // Özellikle ilkel tipler için değer kopyalaması yapılmalıdır.
    // Birleşik tipler için derin kopya gerekebilir.
    // Bu kısım dilin semantiğine göre daha da geliştirilmelidir.
    var->value = (void*)new_value;
    return true;
}

const void* variable_get_value(const Variable* var) {
    return var->value;
}

void variable_free(Variable* var) {
    if (var == NULL) {
        return;
    }
    free(var->name);
    var->type
    var->value
    free(var);
}

void variable_print(const Variable* var) {
    if (var == NULL) {
        printf("Değişken: NULL\n");
        return;
    }
    printf("Değişken: Ad=%s, Tip=", var->name);
    if (var->type) {
        // type_print fonksiyonu cnt_type.c dosyasında tanımlanmış olmalı
        // type_print(var->type);
        printf("%s", var->type->name); // Basitçe tip adını yazdırıyoruz
    } else {
        printf("Bilinmiyor");
    }
    printf(", Değiştirilebilir=%s, Değer Adresi=%p\n", var->is_mutable ? "Evet" : "Hayır", var->value);
}