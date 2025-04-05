#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cnt_xml.h"

// Yardımcı Fonksiyonlar (Basit Ayrıştırma)

static char* xml_read_tag_name(const char* xml_string, size_t* index) {
    // ... diziden etiket adı okuma ...
}

static char* xml_read_text_content(const char* xml_string, size_t* index) {
    // ... diziden metin içeriği okuma ...
}

static XmlAttribute* xml_read_attributes(const char* xml_string, size_t* index) {
    // ... diziden özellikler okuma ...
}

static XmlNode* xml_read_node(const char* xml_string, size_t* index) {
    // ... diziden XML düğümü okuma ...
}

// XML Ayrıştırma Fonksiyonu
XmlNode* xml_parse(const char* xml_string) {
    size_t index = 0;
    return xml_read_node(xml_string, &index);
}

// Yardımcı Fonksiyonlar (Basit Oluşturma)

static char* xml_stringify_attributes(XmlAttribute* attributes) {
    // ... XML özelliklerini dizeye dönüştürme ...
}

static char* xml_stringify_node(XmlNode* xml_node) {
    // ... XML düğümünü dizeye dönüştürme ...
}

// XML Oluşturma Fonksiyonu
char* xml_stringify(XmlNode* xml_node) {
    return xml_stringify_node(xml_node);
}

// XML Düğümünü Serbest Bırakma Fonksiyonu
void free_xml_node(XmlNode* xml_node) {
    // ... XML düğümünün belleğini serbest bırakma ...
}