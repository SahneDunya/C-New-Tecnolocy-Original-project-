#include "cnt_function_signature.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

FunctionSignature* function_signature_create(const char* name, FunctionParameter* parameters) {
    FunctionSignature* signature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    if (!signature) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    signature->name = strdup(name);
    signature->parameter_count = 0;
    FunctionParameter* current = parameters;
    while (current != NULL) {
        signature->parameter_count++;
        current = current->next;
    }
    signature->parameter_types = (Type**)malloc(sizeof(Type*) * signature->parameter_count);
    if (!signature->parameter_types) {
        perror("Bellek ayırma hatası");
        free(signature->name);
        free(signature);
        exit(EXIT_FAILURE);
    }
    current = parameters;
    for (int i = signature->parameter_count - 1; i >= 0; --i) {
        signature->parameter_types[i] = current->type;
        current = current->next;
    }
    return signature;
}

bool function_signatures_equal(const FunctionSignature* sig1, const FunctionSignature* sig2) {
    if (sig1 == NULL || sig2 == NULL) {
        return false;
    }
    if (strcmp(sig1->name, sig2->name) != 0) {
        return false;
    }
    if (sig1->parameter_count != sig2->parameter_count) {
        return false;
    }
    for (int i = 0; i < sig1->parameter_count; ++i) {
        if (!types_are_equal(sig1->parameter_types[i], sig2->parameter_types[i])) {
            return false;
        }
    }
    return true;
}

void function_signature_print(const FunctionSignature* signature) {
    if (!signature) {
        printf("Fonksiyon İmzası: NULL\n");
        return;
    }
    printf("İmza: %s(", signature->name);
    for (int i = 0; i < signature->parameter_count; ++i) {
        printf("%s", signature->parameter_types[i]->name);
        if (i < signature->parameter_count - 1) {
            printf(", ");
        }
    }
    printf(")\n");
}