#ifndef CNT_SEMANTIC_ANALYZER_H
#define CNT_SEMANTIC_ANALYZER_H

#include "cnt_ast.h"
#include "cnt_symboltable.h"
#include "cnt_type.h" // Tipleri temsil etmek için

// Semantik Analizci Yapısı
typedef struct {
    Program* ast;
    SymbolTable* global_scope;
    // ... diğer analizci durumu ...
} SemanticAnalyzer;

// Semantik Analizi Başlatma Fonksiyonu
SemanticAnalyzer* semantic_analyzer_create(Program* ast);

// Programın Semantik Analizini Yapma Fonksiyonu
void analyze_program(SemanticAnalyzer* analyzer);

// Bildirimleri Analiz Etme Fonksiyonları
void analyze_declaration(SemanticAnalyzer* analyzer, Declaration* declaration);
void analyze_function_declaration(SemanticAnalyzer* analyzer, Declaration* function_declaration);
void analyze_variable_declaration(SemanticAnalyzer* analyzer, Declaration* variable_declaration);
void analyze_module_declaration(SemanticAnalyzer* analyzer, Declaration* module_declaration);
void analyze_interface_declaration(SemanticAnalyzer* analyzer, Declaration* interface_declaration);
void analyze_implementation_block(SemanticAnalyzer* analyzer, Declaration* implementation_block);

// Deyimleri Analiz Etme Fonksiyonları
void analyze_statement(SemanticAnalyzer* analyzer, Statement* statement, SymbolTable* current_scope);
void analyze_block_statement(SemanticAnalyzer* analyzer, Statement* block_statement, SymbolTable* parent_scope);
void analyze_if_statement(SemanticAnalyzer* analyzer, Statement* if_statement, SymbolTable* current_scope);
void analyze_while_statement(SemanticAnalyzer* analyzer, Statement* while_statement, SymbolTable* current_scope);
void analyze_for_statement(SemanticAnalyzer* analyzer, Statement* for_statement, SymbolTable* current_scope);
void analyze_return_statement(SemanticAnalyzer* analyzer, Statement* return_statement, SymbolTable* current_scope, Type* expected_return_type);
void analyze_expression_statement(SemanticAnalyzer* analyzer, Statement* expression_statement, SymbolTable* current_scope);

// İfadeleri Analiz Etme Fonksiyonları
Type* analyze_expression(SemanticAnalyzer* analyzer, Expression* expression, SymbolTable* current_scope);
Type* analyze_binary_expression(SemanticAnalyzer* analyzer, Expression* binary_expression, SymbolTable* current_scope);
Type* analyze_unary_expression(SemanticAnalyzer* analyzer, Expression* unary_expression, SymbolTable* current_scope);
Type* analyze_call_expression(SemanticAnalyzer* analyzer, Expression* call_expression, SymbolTable* current_scope);
Type* analyze_identifier_expression(SemanticAnalyzer* analyzer, Expression* identifier_expression, SymbolTable* current_scope);
Type* analyze_literal_expression(SemanticAnalyzer* analyzer, Expression* literal_expression);

// Hata Raporlama Fonksiyonu
void semantic_error(size_t line, size_t column, const char* message);

#endif