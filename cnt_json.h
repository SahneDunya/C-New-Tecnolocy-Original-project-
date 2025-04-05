#ifndef CNT_JSON_H
#define CNT_JSON_H

#include <stdbool.h>
#include <stddef.h>

// JSON Veri Tipi
typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

// JSON Değer Yapısı
typedef struct {
    JsonType type;
    union {
        bool boolean;
        double number;
        char* string;
        struct JsonValue** array;
        struct JsonObject* object;
    } value;
} JsonValue;

// JSON Nesne Yapısı
typedef struct JsonObject {
    char* key;
    JsonValue* value;
    struct JsonObject* next;
} JsonObject;

// JSON Ayrıştırma Fonksiyonu
JsonValue* json_parse(const char* json_string);

// JSON Oluşturma Fonksiyonu
char* json_stringify(JsonValue* json_value);

// JSON Değerini Serbest Bırakma Fonksiyonu
void free_json_value(JsonValue* json_value);

#endif