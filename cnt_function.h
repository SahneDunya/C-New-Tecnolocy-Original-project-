#ifndef CNT_FUNCTION_H
#define CNT_FUNCTION_H

#include <stdbool.h>
#include "cnt_ast.h"          // Temel AST düğümü yapısı için (ASTNode)
#include "cnt_type.h"         // Fonksiyonun dönüş tipi ve parametre tipleri için (Type)
#include "cnt_symboltable.h"  // Fonksiyonun tanımlandığı kapsam için (SymbolTable)
#include "cnt_statement.h"    // Fonksiyonun gövdesi için (StatementListNode)
#include "cnt_function_parameter.h" // Fonksiyon parametreleri için (FunctionParameter)

// Fonksiyon AST Düğümü
typedef struct FunctionNode {
    ASTNode base;
    char* name;             // Fonksiyonun adı
    Type* return_type;      // Fonksiyonun dönüş tipi
    FunctionParameter* parameters; // Fonksiyonun parametre listesi (bağlı liste olabilir)
    StatementListNode* body;    // Fonksiyonun gövdesi
    SymbolTable* scope;         // Fonksiyonun tanımlandığı kapsam
    bool is_static;         // Modern özellik: static fonksiyon mu?
    // İleride eklenebilecek diğer özellikler (örneğin, is_inline)
} FunctionNode;

// Yeni bir fonksiyon AST düğümü oluşturma fonksiyonu
FunctionNode* function_create(const char* name, Type* return_type, FunctionParameter* parameters, StatementListNode* body, SymbolTable* scope, bool is_static);

// Fonksiyonun parametre listesine parametre ekleme fonksiyonu
void function_add_parameter(FunctionNode* function, FunctionParameter* parameter);

// Bir fonksiyonun imzasını (ad ve parametre tipleri) yazdırma (debug için)
void function_print_signature(const FunctionNode* function);

#endif