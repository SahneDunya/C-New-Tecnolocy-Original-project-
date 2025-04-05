#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_database.h"
#include <sqlite3.h>

// Veritabanı Bağlantısı Oluşturma Fonksiyonu
Database* database_connect(const char* filename) {
    Database* db = (Database*)malloc(sizeof(Database));
    if (sqlite3_open(filename, &db->db) != SQLITE_OK) {
        free(db);
        return NULL;
    }
    return db;
}

// Veritabanı Sorgusu Çalıştırma Fonksiyonu
bool database_execute(Database* db, const char* sql) {
    char* error_message = NULL;
    if (sqlite3_exec(db->db, sql, NULL, NULL, &error_message) != SQLITE_OK) {
        sqlite3_free(error_message);
        return false;
    }
    return true;
}

// Veritabanı Sorgusu Sonuçlarını Alma Fonksiyonu
char** database_query(Database* db, const char* sql, int* rows, int* columns) {
    char** result = NULL;
    int rc = sqlite3_get_table(db->db, sql, &result, rows, columns, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }
    return result;
}

// Veritabanı Bağlantısını Kapatma Fonksiyonu
bool database_close(Database* db) {
    return sqlite3_close(db->db) == SQLITE_OK;
}

// Veritabanı Serbest Bırakma Fonksiyonu
void free_database(Database* db) {
    sqlite3_close(db->db);
    free(db);
}