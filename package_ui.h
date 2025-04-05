#ifndef PACKAGE_UI_H
#define PACKAGE_UI_H

#include <stdbool.h>
#include <stddef.h>
#include "package_manager.h" // Paket yöneticisi işlevleri için

// Kullanıcı Arayüzü Başlatma Fonksiyonu
void package_ui_init(PackageManager* manager);

// Kullanıcı Arayüzü Döngüsü Fonksiyonu
void package_ui_loop(PackageManager* manager);

// Kullanıcı Arayüzü Temizleme Fonksiyonu
void package_ui_cleanup();

#endif