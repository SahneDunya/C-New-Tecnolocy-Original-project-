#include "cnt_function_parameter.h"
#include <stdlib.h>
#include <string.h>

FunctionParameter* function_parameter_create(const char* name, Type* type, bool has_default_value, bool is_variadic) {
    FunctionParameter* parameter = (FunctionParameter*)malloc(sizeof(FunctionParameter));
    if (!parameter) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    parameter->name = strdup(name);
    parameter->type = type;
    parameter->has_default_value = has_default_value;
    parameter->is_variadic = is_variadic;
    parameter->next = NULL;
    return parameter;
}

 void function_parameter_set_default_value(FunctionParameter* parameter, ExpressionNode* defaultValue) {
     if (parameter) {
         parameter->has_default_value = true;
         parameter->default_value = defaultValue;
     }
 }