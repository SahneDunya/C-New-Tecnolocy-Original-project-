#ifndef CNT_XML_H
#define CNT_XML_H

#include <stdbool.h>
#include <stddef.h>

// XML Düğüm Yapısı
typedef struct XmlNode {
    char* tag_name;
    char* text_content;
    struct XmlAttribute* attributes;
    struct XmlNode* children;
    struct XmlNode* next_sibling;
} XmlNode;

// XML Özellik Yapısı
typedef struct XmlAttribute {
    char* name;
    char* value;
    struct XmlAttribute* next;
} XmlAttribute;

// XML Ayrıştırma Fonksiyonu
XmlNode* xml_parse(const char* xml_string);

// XML Oluşturma Fonksiyonu
char* xml_stringify(XmlNode* xml_node);

// XML Düğümünü Serbest Bırakma Fonksiyonu
void free_xml_node(XmlNode* xml_node);

#endif