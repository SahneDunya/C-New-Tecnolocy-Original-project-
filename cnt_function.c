#include "cnt_function.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

FunctionNode* function_create(const char* name, Type* return_type, FunctionParameter* parameters, StatementListNode* body, SymbolTable* scope, bool is_static) {
    FunctionNode* node = (FunctionNode*)malloc(sizeof(FunctionNode));
    if (!node) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    node->base.type = AST_TYPE_FUNCTION_DEFINITION;
    node->name = strdup(name);
    node->return_type = return_type;
    node->parameters = parameters;
    node->body = body;
    node->scope = scope;
    node->is_static = is_static;
    return node;
}

void function_add_parameter(FunctionNode* function, FunctionParameter* parameter) {
    if (!function || !parameter) {
        return;
    }
    parameter->next = function->parameters;
    function->parameters = parameter;
}

void function_print_signature(const FunctionNode* function) {
    if (!function) {
        printf("Fonksiyon: NULL\n");
        return;
    }
    printf("Fonksiyon Adı: %s, Dönüş Tipi: ", function->name);
    if (function->return_type) {
        printf("%s", function->return_type->name);
    } else {
        printf("void");
    }
    printf(", Parametreler: (");
    FunctionParameter* current = function->parameters;
    bool first = true;
    while (current != NULL) {
        if (!first) {
            printf(", ");
        }
        printf("%s %s", current->type->name, current->name);
        current = current->next;
        first = false;
    }
    printf(")\n");
}