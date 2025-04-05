#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "package_ui.h"

// Yardımcı Fonksiyonlar (Basit CLI)

static void print_help() {
    printf("Kullanım:\n");
    printf("  cntpm install <paket_adı>\n");
    printf("  cntpm uninstall <paket_adı>\n");
    printf("  cntpm update <paket_adı>\n");
    printf("  cntpm search <sorgu>\n");
    printf("  cntpm list\n");
    printf("  cntpm help\n");
}

static void handle_install(PackageManager* manager, char* package_name) {
    if (package_manager_install(manager, package_name)) {
        printf("%s başarıyla yüklendi.\n", package_name);
    } else {
        printf("%s yüklenirken bir hata oluştu.\n", package_name);
    }
}

static void handle_uninstall(PackageManager* manager, char* package_name) {
    if (package_manager_uninstall(manager, package_name)) {
        printf("%s başarıyla kaldırıldı.\n", package_name);
    } else {
        printf("%s kaldırılırken bir hata oluştu.\n", package_name);
    }
}

static void handle_update(PackageManager* manager, char* package_name) {
    if (package_manager_update(manager, package_name)) {
        printf("%s başarıyla güncellendi.\n", package_name);
    } else {
        printf("%s güncellenirken bir hata oluştu.\n", package_name);
    }
}

static void handle_search(PackageManager* manager, char* query) {
    size_t result_count;
    PackageConfig** results = package_manager_search(manager, query, &result_count);
    if (results == NULL) {
        printf("Arama sonuçları bulunamadı.\n");
        return;
    }

    printf("Arama sonuçları:\n");
    for (size_t i = 0; i < result_count; i++) {
        printf("  %s (%s)\n", results[i]->name, results[i]->version);
    }
}

static void handle_list(PackageManager* manager) {
    size_t package_count;
    PackageConfig** packages = package_manager_list_installed(manager, &package_count);
    if (packages == NULL) {
        printf("Yüklü paket bulunamadı.\n");
        return;
    }

    printf("Yüklü paketler:\n");
    for (size_t i = 0; i < package_count; i++) {
        printf("  %s (%s)\n", packages[i]->name, packages[i]->version);
    }
}

// Kullanıcı Arayüzü Başlatma Fonksiyonu
void package_ui_init(PackageManager* manager) {
    // ... başlatma işlemleri ...
}

// Kullanıcı Arayüzü Döngüsü Fonksiyonu
void package_ui_loop(PackageManager* manager) {
    char command[1024];
    while (true) {
        printf("cntpm> ");
        fgets(command, sizeof(command), stdin);

        char* token = strtok(command, " \n");
        if (token == NULL) {
            continue;
        }

        if (strcmp(token, "install") == 0) {
            token = strtok(NULL, " \n");
            if (token == NULL) {
                print_help();
                continue;
            }
            handle_install(manager, token);
        } else if (strcmp(token, "uninstall") == 0) {
            token = strtok(NULL, " \n");
            if (token == NULL) {
                print_help();
                continue;
            }
            handle_uninstall(manager, token);
        } else if (strcmp(token, "update") == 0) {
            token = strtok(NULL, " \n");
            if (token == NULL) {
                print_help();
                continue;
            }
            handle_update(manager, token);
        } else if (strcmp(token, "search") == 0) {
            token = strtok(NULL, " \n");
            if (token == NULL) {
                print_help();
                continue;
            }
            handle_search(manager, token);
        } else if (strcmp(token, "list") == 0) {
            handle_list(manager);
        } else if (strcmp(token, "help") == 0) {
            print_help();
        } else if (strcmp(token, "exit") == 0) {
            break;
        } else {
            print_help();
        }
    }
}

// Kullanıcı Arayüzü Temizleme Fonksiyonu
void package_ui_cleanup() {
    // ... temizleme işlemleri ...
}