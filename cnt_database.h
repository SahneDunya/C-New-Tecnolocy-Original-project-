#ifndef CNT_DATABASE_H
#define CNT_DATABASE_H

#include <stdbool.h>
#include <stddef.h>

// Veritabanı Yapısı
typedef struct {
    sqlite3* db;
} Database;

// Veritabanı Bağlantısı Oluşturma Fonksiyonu
Database* database_connect(const char* filename);

// Veritabanı Sorgusu Çalıştırma Fonksiyonu
bool database_execute(Database* db, const char* sql);

// Veritabanı Sorgusu Sonuçlarını Alma Fonksiyonu
char** database_query(Database* db, const char* sql, int* rows, int* columns);

// Veritabanı Bağlantısını Kapatma Fonksiyonu
bool database_close(Database* db);

// Veritabanı Serbest Bırakma Fonksiyonu
void free_database(Database* db);

#endif