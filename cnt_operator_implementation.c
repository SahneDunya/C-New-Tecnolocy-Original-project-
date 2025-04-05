#include "cnt_operator_implementation.h"
#include "cnt_primitive_types.h" // cnt_int, cnt_float, cnt_bool için
#include <stdlib.h>
#include <stdio.h>

// Yardımcı fonksiyon: Bellekte yeni bir değer oluşturur ve pointer'ını döndürür
void* allocate_and_assign(const void* value, size_t size) {
    void* result = malloc(size);
    if (result == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    memcpy(result, value, size);
    return result;
}

// Aritmetik Operatör Implementasyonları (Tam Sayılar)
void* add_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    cnt_int result = l + r;
    *result_type = type_create(TYPE_INT, "int");
    return allocate_and_assign(&result, sizeof(cnt_int));
}

void* subtract_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    cnt_int result = l - r;
    *result_type = type_create(TYPE_INT, "int");
    return allocate_and_assign(&result, sizeof(cnt_int));
}

void* multiply_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    cnt_int result = l * r;
    *result_type = type_create(TYPE_INT, "int");
    return allocate_and_assign(&result, sizeof(cnt_int));
}

void* divide_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    if (r == 0) {
        fprintf(stderr, "Hata: Sıfıra bölme.\n");
        exit(EXIT_FAILURE);
    }
    cnt_int result = l / r;
    *result_type = type_create(TYPE_INT, "int");
    return allocate_and_assign(&result, sizeof(cnt_int));
}

void* modulo_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    if (r == 0) {
        fprintf(stderr, "Hata: Sıfıra bölme (mod alma).\n");
        exit(EXIT_FAILURE);
    }
    cnt_int result = l % r;
    *result_type = type_create(TYPE_INT, "int");
    return allocate_and_assign(&result, sizeof(cnt_int));
}

// Aritmetik Operatör Implementasyonları (Kayan Noktalı Sayılar)
void* add_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_float l = *(const cnt_float*)left;
    cnt_float r = *(const cnt_float*)right;
    cnt_float result = l + r;
    *result_type = type_create(TYPE_FLOAT, "float");
    return allocate_and_assign(&result, sizeof(cnt_float));
}

void* subtract_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_float l = *(const cnt_float*)left;
    cnt_float r = *(const cnt_float*)right;
    cnt_float result = l - r;
    *result_type = type_create(TYPE_FLOAT, "float");
    return allocate_and_assign(&result, sizeof(cnt_float));
}

void* multiply_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_float l = *(const cnt_float*)left;
    cnt_float r = *(const cnt_float*)right;
    cnt_float result = l * r;
    *result_type = type_create(TYPE_FLOAT, "float");
    return allocate_and_assign(&result, sizeof(cnt_float));
}

void* divide_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_float l = *(const cnt_float*)left;
    cnt_float r = *(const cnt_float*)right;
    cnt_float result = l / r;
    *result_type = type_create(TYPE_FLOAT, "float");
    return allocate_and_assign(&result, sizeof(cnt_float));
}

// Karşılaştırma Operatör Implementasyonları (Tam Sayılar)
void* equals_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    cnt_bool result = (l == r);
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

void* not_equals_int(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_int l = *(const cnt_int*)left;
    cnt_int r = *(const cnt_int*)right;
    cnt_bool result = (l != r);
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

// ... diğer karşılaştırma operatörleri (greater than, less than vb.) tam sayılar için benzer şekilde implemente edilebilir ...

// Karşılaştırma Operatör Implementasyonları (Kayan Noktalı Sayılar)
void* equals_float(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_float l = *(const cnt_float*)left;
    cnt_float r = *(const cnt_float*)right;
    cnt_bool result = (l == r);
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

// ... diğer karşılaştırma operatörleri (greater than, less than vb.) kayan noktalı sayılar için benzer şekilde implemente edilebilir ...

// Mantıksal Operatör Implementasyonları (Boolean)
void* and_bool(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_bool l = *(const cnt_bool*)left;
    cnt_bool r = *(const cnt_bool*)right;
    cnt_bool result = (l && r);
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

void* or_bool(const void* left, const Type* left_type, const void* right, const Type* right_type, Type** result_type) {
    cnt_bool l = *(const cnt_bool*)left;
    cnt_bool r = *(const cnt_bool*)right;
    cnt_bool result = (l || r);
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

void* not_bool(const void* operand, const Type* operand_type, Type** result_type) {
    cnt_bool op = *(const cnt_bool*)operand;
    cnt_bool result = !op;
    *result_type = type_create(TYPE_BOOL, "bool");
    return allocate_and_assign(&result, sizeof(cnt_bool));
}

// ... diğer operatörler (bitwise, assignment vb.) ve tipler için implementasyonlar ...