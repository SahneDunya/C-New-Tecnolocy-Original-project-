#ifndef CNT_AST_H
#define CNT_AST_H

#include <stdbool.h>
#include <stddef.h>

// İleriye Yönelik Bildirimler (Gerekli olabilecek diğer AST düğümleri için)
typedef struct ASTNode_s ASTNode;
typedef struct Expression_s Expression;
typedef struct Statement_s Statement;
typedef struct Declaration_s Declaration;
typedef struct TypeSpecifier_s TypeSpecifier;

// AST Düğüm Türleri
typedef enum {
    AST_PROGRAM,
    AST_MODULE_DECLARATION,
    AST_IMPORT_STATEMENT,
    AST_INTERFACE_DECLARATION,
    AST_IMPLEMENTATION_BLOCK,
    AST_FUNCTION_DECLARATION,
    AST_VARIABLE_DECLARATION,
    AST_TYPE_SPECIFIER,
    AST_BLOCK_STATEMENT,
    AST_IF_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_FOR_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_EXPRESSION_STATEMENT,
    AST_BINARY_EXPRESSION,
    AST_UNARY_EXPRESSION,
    AST_CALL_EXPRESSION,
    AST_IDENTIFIER_EXPRESSION,
    AST_INTEGER_LITERAL_EXPRESSION,
    AST_FLOAT_LITERAL_EXPRESSION,
    AST_STRING_LITERAL_EXPRESSION,
    // ... diğer AST düğüm türleri ...
} ASTNodeType;

// Temel AST Düğümü Yapısı
struct ASTNode_s {
    ASTNodeType type;
    size_t line;
    size_t column;
    // Ortak bilgiler buraya eklenebilir (örneğin, çocuklar için bir liste)
    struct ASTNode_s* next; // Aynı seviyedeki düğümleri bağlamak için (isteğe bağlı)
};

// İfade Düğümleri
struct Expression_s {
    ASTNodeType type; // Expression türü her zaman AST_..._EXPRESSION olmalı
    size_t line;
    size_t column;
    // İfadeye özgü bilgiler
    union {
        struct {
            Expression* left;
            char* operator;
            Expression* right;
        } binary;
        struct {
            char* operator;
            Expression* operand;
        } unary;
        struct {
            Expression* function;
            ASTNode* arguments; // İfade listesi olabilir
        } call;
        char* identifier;
        long long int_value;
        double float_value;
        char* string_value;
    } value;
};

// Deyim Düğümleri
struct Statement_s {
    ASTNodeType type; // Statement türü her zaman AST_..._STATEMENT olmalı
    size_t line;
    size_t column;
    // Deyime özgü bilgiler
    union {
        struct {
            ASTNode* statements; // Bir blok içindeki deyimlerin listesi
        } block;
        struct {
            Expression* condition;
            Statement* then_branch;
            Statement* else_branch; // İsteğe bağlı
        } if_stmt;
        struct {
            Expression* condition;
            Statement* body;
        } while_stmt;
        struct {
            Statement* init;
            Expression* condition;
            Expression* increment;
            Statement* body;
        } for_stmt;
        struct {
            Expression* value; // İsteğe bağlı dönüş değeri
        } return_stmt;
        Expression* expression; // İfade deyimi
    } content;
};

// Bildirim Düğümleri
struct Declaration_s {
    ASTNodeType type; // Declaration türü her zaman AST_..._DECLARATION olmalı
    size_t line;
    size_t column;
    // Bildirime özgü bilgiler
    union {
        struct {
            char* name;
            Statement* body; // Fonksiyon gövdesi (blok deyimi)
            TypeSpecifier* return_type;
            ASTNode* parameters; // Değişken bildirimi listesi olabilir
        } function;
        struct {
            char* name;
            TypeSpecifier* type;
            Expression* initializer; // İsteğe bağlı başlangıç değeri
        } variable;
        struct {
            char* name;
            // ... modül içeriği (bildirimler, ifadeler) ...
            ASTNode* body; // Blok benzeri bir yapı olabilir
        } module;
        struct {
            char* name;
            // ... arayüz üyeleri (fonksiyon prototipleri) ...
            ASTNode* members; // Fonksiyon bildirimi listesi olabilir
        } interface;
        struct {
            const Interface* interface;
            void* object; // Arayüzü uygulayan nesneye işaretçi (derleme zamanında bilinmeyebilir)
            ASTNode* methods; // Fonksiyon bildirimi listesi (uygulamalar)
        } implementation;
    } data;
};

// Tip Belirleyici Düğümü
struct TypeSpecifier_s {
    ASTNodeType type; // Her zaman AST_TYPE_SPECIFIER
    size_t line;
    size_t column;
    char* name; // Örneğin, "int", "char", "float" veya özel tip adı
    // ... ek tip bilgileri (örneğin, pointer derinliği) ...
};

// Program Kök Düğümü
typedef struct {
    ASTNode base; // AST_PROGRAM türünde
    ASTNode* body; // Üst düzey bildirimlerin (modüller, fonksiyonlar vb.) listesi
} Program;

#endif