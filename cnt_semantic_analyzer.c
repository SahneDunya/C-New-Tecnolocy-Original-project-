#include "cnt_semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Geçerli hata sayacı
static int error_count = 0;

SemanticAnalyzer* semantic_analyzer_create(Program* ast) {
    SemanticAnalyzer* analyzer = (SemanticAnalyzer*)malloc(sizeof(SemanticAnalyzer));
    if (analyzer == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    analyzer->ast = ast;
    analyzer->global_scope = symboltable_create(NULL);
    return analyzer;
}

void semantic_error(size_t line, size_t column, const char* message) {
    fprintf(stderr, "Semantik Hata satır %zu, sütun %zu: %s\n", line, column, message);
    error_count++;
}

void analyze_program(SemanticAnalyzer* analyzer) {
    // Önce yerleşik tipleri sembol tablosuna ekleyebiliriz
    symboltable_add(analyzer->global_scope, "int", type_create("int"), SCOPE_GLOBAL);
    symboltable_add(analyzer->global_scope, "float", type_create("float"), SCOPE_GLOBAL);
    symboltable_add(analyzer->global_scope, "bool", type_create("bool"), SCOPE_GLOBAL);
    symboltable_add(analyzer->global_scope, "void", type_create("void"), SCOPE_GLOBAL);

    ASTNode* current = analyzer->ast->body;
    while (current != NULL) {
        analyze_declaration(analyzer, (Declaration*)current);
        current = current->next;
    }

    if (error_count > 0) {
        fprintf(stderr, "%d semantik hata bulundu.\n", error_count);
    } else {
        printf("Semantik analiz başarıyla tamamlandı.\n");
    }
}

void analyze_declaration(SemanticAnalyzer* analyzer, Declaration* declaration) {
    switch (declaration->base.type) {
        case AST_FUNCTION_DECLARATION:
            analyze_function_declaration(analyzer, declaration);
            break;
        case AST_VARIABLE_DECLARATION:
            analyze_variable_declaration(analyzer, declaration);
            break;
        case AST_MODULE_DECLARATION:
            analyze_module_declaration(analyzer, declaration);
            break;
        case AST_INTERFACE_DECLARATION:
            analyze_interface_declaration(analyzer, declaration);
            break;
        case AST_IMPLEMENTATION_BLOCK:
            analyze_implementation_block(analyzer, declaration);
            break;
        default:
            semantic_error(declaration->base.line, declaration->base.column, "Bilinmeyen bildirim türü");
            break;
    }
}

void analyze_function_declaration(SemanticAnalyzer* analyzer, Declaration* function_declaration) {
    // Fonksiyon adını sembol tablosuna ekle
    Type* return_type = function_declaration->data.function.return_type ? type_create(function_declaration->data.function.return_type->name) : type_create("void"); // Basit dönüş tipi oluşturma
    symboltable_add(analyzer->global_scope, function_declaration->data.function.name, return_type, SCOPE_GLOBAL);

    // Yeni bir kapsam oluştur
    SymbolTable* function_scope = symboltable_create(analyzer->global_scope);

    // Parametreleri sembol tablosuna ekle (şimdilik boş)
    ASTNode* param = function_declaration->data.function.parameters;
    while (param != NULL) {
        // ... parametre analizi ve sembol tablosuna ekleme ...
        param = param->next;
    }

    // Fonksiyon gövdesini analiz et
    analyze_statement(analyzer, function_declaration->data.function.body, function_scope);

    symboltable_free(function_scope);
}

void analyze_variable_declaration(SemanticAnalyzer* analyzer, Declaration* variable_declaration) {
    Type* var_type = type_create(variable_declaration->data.variable.type->name); // Basit tip oluşturma
    if (symboltable_find_local(analyzer->global_scope, variable_declaration->data.variable.name) != NULL) {
        semantic_error(variable_declaration->base.line, variable_declaration->base.column, "Değişken zaten tanımlanmış");
        return;
    }
    symboltable_add(analyzer->global_scope, variable_declaration->data.variable.name, var_type, SCOPE_GLOBAL);

    if (variable_declaration->data.variable.initializer != NULL) {
        Type* initializer_type = analyze_expression(analyzer, variable_declaration->data.variable.initializer, analyzer->global_scope);
        if (!types_are_compatible(var_type, initializer_type)) {
            semantic_error(variable_declaration->base.line, variable_declaration->base.column, "Başlatıcı ifadesinin tipi değişkenin tipiyle uyuşmuyor");
        }
    }
}

void analyze_module_declaration(SemanticAnalyzer* analyzer, Declaration* module_declaration) {
    // Modül adını sembol tablosuna ekle (şimdilik basit bir işaretleyici olarak)
    symboltable_add(analyzer->global_scope, module_declaration->data.module.name, type_create("module"), SCOPE_MODULE);

    // Yeni bir kapsam oluştur
    SymbolTable* module_scope = symboltable_create(analyzer->global_scope);

    // Modül gövdesini analiz et
    Statement* body = (Statement*)module_declaration->data.module.body;
    if (body != NULL && body->base.type == AST_BLOCK_STATEMENT) {
        ASTNode* current = body->content.block.statements;
        while (current != NULL) {
            analyze_declaration(analyzer, (Declaration*)current); // Modül içinde de bildirimler olabilir
            current = current->next;
        }
    }

    symboltable_free(module_scope);
}

void analyze_interface_declaration(SemanticAnalyzer* analyzer, Declaration* interface_declaration) {
    // Arayüz adını sembol tablosuna ekle (şimdilik basit bir işaretleyici olarak)
    symboltable_add(analyzer->global_scope, interface_declaration->data.interface.name, type_create("interface"), SCOPE_GLOBAL);

    // Yeni bir kapsam oluştur
    SymbolTable* interface_scope = symboltable_create(analyzer->global_scope);

    // Arayüz üyelerini analiz et (şimdilik sadece isimleri kontrol ediyoruz)
    Statement* body = (Statement*)interface_declaration->data.interface.members;
    if (body != NULL && body->base.type == AST_BLOCK_STATEMENT) {
        ASTNode* current = body->content.block.statements;
        while (current != NULL) {
            if (((Declaration*)current)->base.type == AST_FUNCTION_DECLARATION) {
                if (symboltable_find_local(interface_scope, ((Declaration*)current)->data.function.name) != NULL) {
                    semantic_error(((Declaration*)current)->base.line, ((Declaration*)current)->base.column, "Arayüzde aynı isimde birden fazla üye tanımlanamaz");
                } else {
                    // Arayüz fonksiyonlarını kapsamına ekle (şimdilik sadece isim)
                    symboltable_add(interface_scope, ((Declaration*)current)->data.function.name, type_create("function"), SCOPE_GLOBAL); // Kapsam global olarak işaretlendi, düzeltilebilir
                }
            } else {
                semantic_error(((Declaration*)current)->base.line, ((Declaration*)current)->base.column, "Arayüz içinde sadece fonksiyon bildirimi olabilir");
            }
            current = current->next;
        }
    }

    symboltable_free(interface_scope);
}

void analyze_implementation_block(SemanticAnalyzer* analyzer, Declaration* implementation_block) {
    // ... implementasyon bloğu analizi (arayüzü kontrol et, metotları kontrol et) ...
    semantic_error(implementation_block->base.line, implementation_block->base.column, "Implementasyon blokları henüz tam olarak desteklenmiyor");
}

void analyze_statement(SemanticAnalyzer* analyzer, Statement* statement, SymbolTable* current_scope) {
    switch (statement->base.type) {
        case AST_BLOCK_STATEMENT:
            analyze_block_statement(analyzer, statement, current_scope);
            break;
        case AST_IF_STATEMENT:
            analyze_if_statement(analyzer, statement, current_scope);
            break;
        case AST_WHILE_STATEMENT:
            analyze_while_statement(analyzer, statement, current_scope);
            break;
        case AST_FOR_STATEMENT:
            analyze_for_statement(analyzer, statement, current_scope);
            break;
        case AST_RETURN_STATEMENT:
            // Fonksiyonun dönüş tipini almamız gerekecek
            analyze_return_statement(analyzer, statement, current_scope, type_create("void")); // Şimdilik varsayılan
            break;
        case AST_EXPRESSION_STATEMENT:
            analyze_expression_statement(analyzer, statement, current_scope);
            break;
        default:
            semantic_error(statement->base.line, statement->base.column, "Bilinmeyen deyim türü");
            break;
    }
}

void analyze_block_statement(SemanticAnalyzer* analyzer, Statement* block_statement, SymbolTable* parent_scope) {
    SymbolTable* block_scope = symboltable_create(parent_scope);
    ASTNode* current = block_statement->content.block.statements;
    while (current != NULL) {
        analyze_statement(analyzer, (Statement*)current, block_scope);
        current = current->next;
    }
    symboltable_free(block_scope);
}

void analyze_if_statement(SemanticAnalyzer* analyzer, Statement* if_statement, SymbolTable* current_scope) {
    Type* condition_type = analyze_expression(analyzer, if_statement->content.if_stmt.condition, current_scope);
    if (condition_type != NULL && strcmp(condition_type->name, "bool") != 0) {
        semantic_error(if_statement->content.if_stmt.condition->line, if_statement->content.if_stmt.condition->column, "If ifadesinin koşulu boolean tipinde olmalıdır");
    }
    analyze_statement(analyzer, if_statement->content.if_stmt.then_branch, current_scope);
    if (if_statement->content.if_stmt.else_branch != NULL) {
        analyze_statement(analyzer, if_statement->content.if_stmt.else_branch, current_scope);
    }
}

void analyze_while_statement(SemanticAnalyzer* analyzer, Statement* while_statement, SymbolTable* current_scope) {
    Type* condition_type = analyze_expression(analyzer, while_statement->content.while_stmt.condition, current_scope);
    if (condition_type != NULL && strcmp(condition_type->name, "bool") != 0) {
        semantic_error(while_statement->content.while_stmt.condition->line, while_statement->content.while_stmt.condition->column, "While ifadesinin koşulu boolean tipinde olmalıdır");
    }
    analyze_statement(analyzer, while_statement->content.while_stmt.body, current_scope);
}

void analyze_for_statement(SemanticAnalyzer* analyzer, Statement* for_statement, SymbolTable* current_scope) {
    analyze_statement(analyzer, for_statement->content.for_stmt.init, current_scope);
    Type* condition_type = analyze_expression(analyzer, for_statement->content.for_stmt.condition, current_scope);
    if (condition_type != NULL && strcmp(condition_type->name, "bool") != 0) {
        semantic_error(for_statement->content.for_stmt.condition->line, for_statement->content.for_stmt.condition->column, "For ifadesinin koşulu boolean tipinde olmalıdır");
    }
    analyze_expression(analyzer, for_statement->content.for_stmt.increment, current_scope);
    analyze_statement(analyzer, for_statement->content.for_stmt.body, current_scope);
}

void analyze_return_statement(SemanticAnalyzer* analyzer, Statement* return_statement, SymbolTable* current_scope, Type* expected_return_type) {
    Type* return_value_type = analyze_expression(analyzer, return_statement->content.return_stmt.value, current_scope);
    if (expected_return_type != NULL && return_value_type != NULL && !types_are_compatible(expected_return_type, return_value_type)) {
        semantic_error(return_statement->base.line, return_statement->base.column, "Dönüş ifadesinin tipi fonksiyonun dönüş tipiyle uyuşmuyor");
    }
}

void analyze_expression_statement(SemanticAnalyzer* analyzer, Statement* expression_statement, SymbolTable* current_scope) {
    analyze_expression(analyzer, expression_statement->content.expression, current_scope);
}

Type* analyze_expression(SemanticAnalyzer* analyzer, Expression* expression, SymbolTable* current_scope) {
    switch (expression->type) {
        case AST_BINARY_EXPRESSION:
            return analyze_binary_expression(analyzer, expression, current_scope);
        case AST_UNARY_EXPRESSION:
            return analyze_unary_expression(analyzer, expression, current_scope);
        case AST_CALL_EXPRESSION:
            return analyze_call_expression(analyzer, expression, current_scope);
        case AST_IDENTIFIER_EXPRESSION:
            return analyze_identifier_expression(analyzer, expression, current_scope);
        case AST_INTEGER_LITERAL_EXPRESSION:
        case AST_FLOAT_LITERAL_EXPRESSION:
        case AST_STRING_LITERAL_EXPRESSION:
            return analyze_literal_expression(analyzer, expression);
        default:
            semantic_error(expression->line, expression->column, "Bilinmeyen ifade türü");
            return NULL;
    }
}

Type* analyze_binary_expression(SemanticAnalyzer* analyzer, Expression* binary_expression, SymbolTable* current_scope) {
    Type* left_type = analyze_expression(analyzer, binary_expression->value.binary.left, current_scope);
    Type* right_type = analyze_expression(analyzer, binary_expression->value.binary.right, current_scope);
    const char* op = binary_expression->value.binary.operator;

    if (left_type == NULL || right_type == NULL) {
        return NULL;
    }

    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
        if (strcmp(left_type->name, "int") == 0 && strcmp(right_type->name, "int") == 0) {
            return type_create("int");
        } else if ((strcmp(left_type->name, "int") == 0 || strcmp(left_type->name, "float") == 0) &&
                   (strcmp(right_type->name, "int") == 0 || strcmp(right_type->name, "float") == 0)) {
            return type_create("float");
        } else {
            semantic_error(binary_expression->line, binary_expression->column, "Bu operatör bu tiplerle kullanılamaz");
            return NULL;
        }
    } else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 || strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
        if (types_are_compatible(left_type, right_type)) {
            return type_create("bool");
        } else {
            semantic_error(binary_expression->line, binary_expression->column, "Karşılaştırma operatörleri uyumlu tiplerle kullanılmalıdır");
            return NULL;
        }
    }
    // ... diğer operatörler ...
    return NULL;
}

Type* analyze_unary_expression(SemanticAnalyzer* analyzer, Expression* unary_expression, SymbolTable* current_scope) {
    Type* operand_type = analyze_expression(analyzer, unary_expression->value.unary.operand, current_scope);
    const char* op = unary_expression->value.unary.operator;

    if (operand_type == NULL) {
        return NULL;
    }

    if (strcmp(op, "-") == 0) {
        if (strcmp(operand_type->name, "int") == 0 || strcmp(operand_type->name, "float") == 0) {
            return operand_type;
        } else {
            semantic_error(unary_expression->line, unary_expression->column, "Tekli eksi operatörü sayısal tiplerle kullanılabilir");
            return NULL;
        }
    } else if (strcmp(op, "!") == 0) {
        if (strcmp(operand_type->name, "bool") == 0) {
            return type_create("bool");
        } else {
            semantic_error(unary_expression->line, unary_expression->column, "Tekli değil operatörü boolean tipiyle kullanılabilir");
            return NULL;
        }
    }
    // ... diğer tekli operatörler ...
    return NULL;
}

Type* analyze_call_expression(SemanticAnalyzer* analyzer, Expression* call_expression, SymbolTable* current_scope) {
    Type* callee_type = analyze_expression(analyzer, call_expression->value.call.function, current_scope);
    if (callee_type != NULL && strcmp(callee_type->name, "function") != 0) {
        semantic_error(call_expression->line, call_expression->column, "Sadece fonksiyonlar çağrılabilir");
        return NULL;
    }
    // ... argümanları kontrol et ...
    return type_create("void"); // Şimdilik fonksiyonların void döndürdüğünü varsayıyoruz
}

Type* analyze_identifier_expression(SemanticAnalyzer* analyzer, Expression* identifier_expression, SymbolTable* current_scope) {
    Symbol* symbol = symboltable_find(current_scope, identifier_expression->value.identifier);
    if (symbol == NULL) {
        semantic_error(identifier_expression->line, identifier_expression->column, "Tanımlayıcı bulunamadı");
        return NULL;
    }
    return symbol->type;
}

Type* analyze_literal_expression(SemanticAnalyzer* analyzer, Expression* literal_expression) {
    switch (literal_expression->type) {
        case AST_INTEGER_LITERAL_EXPRESSION:
            return type_create("int");
        case AST_FLOAT_LITERAL_EXPRESSION:
            return type_create("float");
        case AST_STRING_LITERAL_EXPRESSION:
            return type_create("string"); // Henüz tanımlanmamış olabilir
        default:
            return NULL;
    }
}

// Yardımcı fonksiyon (cnt_type.h ve cnt_type.c dosyalarında tanımlanması gerekir)
bool types_are_compatible(Type* type1, Type* type2) {
    if (type1 == NULL || type2 == NULL) return false;
    return strcmp(type1->name, type2->name) == 0;
}

// Yardımcı fonksiyon (cnt_type.h ve cnt_type.c dosyalarında tanımlanması gerekir)
Type* type_create(const char* name) {
    Type* type = (Type*)malloc(sizeof(Type));
    if (type == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    type->name = strdup(name);
    return type;
}