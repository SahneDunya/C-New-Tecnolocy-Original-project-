#include "cnt_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser* parser_init(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (parser == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    return parser;
}

void parser_consume(Parser* parser) {
    token_free(parser->current_token);
    parser->current_token = lexer_get_next_token(parser->lexer);
}

bool parser_check_token(Parser* parser, TokenType type) {
    return parser->current_token->type == type;
}

void parser_expect_token(Parser* parser, TokenType type, const char* expected_value) {
    if (!parser_check_token(parser, type) || (expected_value != NULL && strcmp(parser->current_token->value, expected_value) != 0)) {
        fprintf(stderr, "Hata satır %zu, sütun %zu: Beklenen token türü %d",
                parser->current_token->line, parser->current_token->column, type);
        if (expected_value != NULL) {
            fprintf(stderr, " (değer: '%s')", expected_value);
        }
        fprintf(stderr, ", ancak '%s' bulundu.\n", parser->current_token->value);
        exit(EXIT_FAILURE);
    }
    parser_consume(parser);
}

void parser_error(Parser* parser, const char* message) {
    fprintf(stderr, "Hata satır %zu, sütun %zu: %s (bulunan token: '%s')\n",
            parser->current_token->line, parser->current_token->column, message, parser->current_token->value);
    exit(EXIT_FAILURE);
}

// Üst Düzey Ayrıştırma
Program* parse_program(Parser* parser) {
    Program* program = (Program*)malloc(sizeof(Program));
    if (program == NULL) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    program->base.type = AST_PROGRAM;
    program->base.line = 1;
    program->base.column = 1;
    program->body = NULL; // Bildirim listesini burada oluşturacağız

    ASTNode* current = NULL;
    ASTNode* head = NULL;

    while (!parser_check_token(parser, TOKEN_EOF)) {
        Declaration* decl = parse_declaration(parser);
        if (decl != NULL) {
            if (head == NULL) {
                head = (ASTNode*)decl;
                current = head;
            } else {
                current->next = (ASTNode*)decl;
                current = current->next;
            }
        } else {
            // Üst düzeyde sadece bildirimlere izin veriyoruz (şimdilik)
            parser_error(parser, "Beklenmeyen token (üst düzeyde sadece bildirimlere izin verilir)");
        }
    }

    program->body = head;
    return program;
}

// Bildirim Ayrıştırma
Declaration* parse_declaration(Parser* parser) {
    if (parser_check_token(parser, TOKEN_KEYWORD)) {
        if (strcmp(parser->current_token->value, "function") == 0) {
            return parse_function_declaration(parser);
        } else if (strcmp(parser->current_token->value, "module") == 0) {
            return parse_module_declaration(parser);
        } else if (strcmp(parser->current_token->value, "interface") == 0) {
            return parse_interface_declaration(parser);
        } else if (strcmp(parser->current_token->value, "impl") == 0) {
            return parse_implementation_block(parser);
        } else {
            // Basit değişken bildirimi varsayalım (örneğin, "int x;")
            TypeSpecifier* type = parse_type_specifier(parser);
            if (parser_check_token(parser, TOKEN_IDENTIFIER)) {
                return parse_variable_declaration(parser, type);
            }
            return NULL;
        }
    }
    return NULL;
}

Declaration* parse_function_declaration(Parser* parser) {
    Declaration* func_decl = (Declaration*)malloc(sizeof(Declaration));
    // ... fonksiyon bildirimi ayrıştırma mantığı ...
    parser_expect_token(parser, TOKEN_KEYWORD, "function");
    if (!parser_check_token(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Fonksiyon adının bekleniyordu");
    }
    func_decl->data.function.name = strdup(parser->current_token->value);
    parser_consume(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "(");
    // ... parametre ayrıştırma ...
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
    // ... dönüş tipi ayrıştırma ...
    func_decl->data.function.body = parse_block_statement(parser);
    func_decl->base.type = AST_FUNCTION_DECLARATION;
    return func_decl;
}

Declaration* parse_variable_declaration(Parser* parser, TypeSpecifier* type) {
    Declaration* var_decl = (Declaration*)malloc(sizeof(Declaration));
    var_decl->data.variable.name = strdup(parser->current_token->value);
    parser_consume(parser);
    var_decl->data.variable.type = type;
    var_decl->data.variable.initializer = NULL; // Şimdilik
    if (parser_check_token(parser, TOKEN_OPERATOR) && strcmp(parser->current_token->value, "=") == 0) {
        parser_consume(parser);
        var_decl->data.variable.initializer = parse_expression(parser);
    }
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ";");
    var_decl->base.type = AST_VARIABLE_DECLARATION;
    return var_decl;
}

Declaration* parse_module_declaration(Parser* parser) {
    Declaration* module_decl = (Declaration*)malloc(sizeof(Declaration));
    parser_expect_token(parser, TOKEN_KEYWORD, "module");
    if (!parser_check_token(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Modül adının bekleniyordu");
    }
    module_decl->data.module.name = strdup(parser->current_token->value);
    parser_consume(parser);
    module_decl->data.module.body = parse_block_statement(parser); // Modül içeriği blok içinde olabilir
    module_decl->base.type = AST_MODULE_DECLARATION;
    return module_decl;
}

Declaration* parse_interface_declaration(Parser* parser) {
    Declaration* interface_decl = (Declaration*)malloc(sizeof(Declaration));
    parser_expect_token(parser, TOKEN_KEYWORD, "interface");
    if (!parser_check_token(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Arayüz adının bekleniyordu");
    }
    interface_decl->data.interface.name = strdup(parser->current_token->value);
    parser_consume(parser);
    interface_decl->data.interface.members = parse_block_statement(parser)->content.block.statements; // Arayüz üyeleri blok içinde olabilir
    interface_decl->base.type = AST_INTERFACE_DECLARATION;
    return interface_decl;
}

Declaration* parse_implementation_block(Parser* parser) {
    Declaration* impl_block = (Declaration*)malloc(sizeof(Declaration));
    parser_expect_token(parser, TOKEN_KEYWORD, "impl");
    // ... implementasyon bloğu ayrıştırma mantığı (arayüz adı, metotlar vb.) ...
    impl_block->base.type = AST_IMPLEMENTATION_BLOCK;
    return impl_block;
}

TypeSpecifier* parse_type_specifier(Parser* parser) {
    TypeSpecifier* type_specifier = (TypeSpecifier*)malloc(sizeof(TypeSpecifier));
    if (parser_check_token(parser, TOKEN_KEYWORD)) {
        type_specifier->name = strdup(parser->current_token->value);
        parser_consume(parser);
        type_specifier->base.type = AST_TYPE_SPECIFIER;
        return type_specifier;
    }
    parser_error(parser, "Tip belirleyici bekleniyordu");
    return NULL; // Erişilmemesi gereken yer
}

// Deyim Ayrıştırma
Statement* parse_statement(Parser* parser) {
    if (parser_check_token(parser, TOKEN_PUNCTUATOR) && strcmp(parser->current_token->value, "{") == 0) {
        return parse_block_statement(parser);
    } else if (parser_check_token(parser, TOKEN_KEYWORD)) {
        if (strcmp(parser->current_token->value, "if") == 0) {
            return parse_if_statement(parser);
        } else if (strcmp(parser->current_token->value, "while") == 0) {
            return parse_while_statement(parser);
        } else if (strcmp(parser->current_token->value, "for") == 0) {
            return parse_for_statement(parser);
        } else if (strcmp(parser->current_token->value, "return") == 0) {
            return parse_return_statement(parser);
        }
    }
    return parse_expression_statement(parser);
}

Statement* parse_block_statement(Parser* parser) {
    Statement* block = (Statement*)malloc(sizeof(Statement));
    block->base.type = AST_BLOCK_STATEMENT;
    block->content.block.statements = NULL;
    ASTNode* head = NULL;
    ASTNode* current = NULL;

    parser_expect_token(parser, TOKEN_PUNCTUATOR, "{");
    while (!parser_check_token(parser, TOKEN_PUNCTUATOR) || strcmp(parser->current_token->value, "}") != 0) {
        Statement* stmt = parse_statement(parser);
        if (stmt != NULL) {
            if (head == NULL) {
                head = (ASTNode*)stmt;
                current = head;
            } else {
                current->next = (ASTNode*)stmt;
                current = current->next;
            }
        } else {
            parser_error(parser, "Beklenen deyim veya '}'");
        }
    }
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "}");
    block->content.block.statements = head;
    return block;
}

Statement* parse_if_statement(Parser* parser) {
    Statement* if_stmt = (Statement*)malloc(sizeof(Statement));
    if_stmt->base.type = AST_IF_STATEMENT;
    parser_expect_token(parser, TOKEN_KEYWORD, "if");
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "(");
    if_stmt->content.if_stmt.condition = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
    if_stmt->content.if_stmt.then_branch = parse_statement(parser);
    if_stmt->content.if_stmt.else_branch = NULL;
    if (parser_check_token(parser, TOKEN_KEYWORD) && strcmp(parser->current_token->value, "else") == 0) {
        parser_consume(parser);
        if_stmt->content.if_stmt.else_branch = parse_statement(parser);
    }
    return if_stmt;
}

Statement* parse_while_statement(Parser* parser) {
    Statement* while_stmt = (Statement*)malloc(sizeof(Statement));
    while_stmt->base.type = AST_WHILE_STATEMENT;
    parser_expect_token(parser, TOKEN_KEYWORD, "while");
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "(");
    while_stmt->content.while_stmt.condition = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
    while_stmt->content.while_stmt.body = parse_statement(parser);
    return while_stmt;
}

Statement* parse_for_statement(Parser* parser) {
    Statement* for_stmt = (Statement*)malloc(sizeof(Statement));
    for_stmt->base.type = AST_FOR_STATEMENT;
    parser_expect_token(parser, TOKEN_KEYWORD, "for");
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "(");
    for_stmt->content.for_stmt.init = parse_expression_statement(parser); // Basit ifade deyimi bekliyoruz
    for_stmt->content.for_stmt.condition = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ";");
    for_stmt->content.for_stmt.increment = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
    for_stmt->content.for_stmt.body = parse_statement(parser);
    return for_stmt;
}

Statement* parse_return_statement(Parser* parser) {
    Statement* return_stmt = (Statement*)malloc(sizeof(Statement));
    return_stmt->base.type = AST_RETURN_STATEMENT;
    parser_expect_token(parser, TOKEN_KEYWORD, "return");
    return_stmt->content.return_stmt.value = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ";");
    return return_stmt;
}

Statement* parse_expression_statement(Parser* parser) {
    Statement* expr_stmt = (Statement*)malloc(sizeof(Statement));
    expr_stmt->base.type = AST_EXPRESSION_STATEMENT;
    expr_stmt->content.expression = parse_expression(parser);
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ";");
    return expr_stmt;
}

// İfade Ayrıştırma (Basit Önceliklendirme ile)
Expression* parse_expression(Parser* parser) {
    return parse_assignment_expression(parser);
}

Expression* parse_assignment_expression(Parser* parser) {
    Expression* left = parse_equality_expression(parser);
    if (parser_check_token(parser, TOKEN_OPERATOR) && strcmp(parser->current_token->value, "=") == 0) {
        Expression* assignment = (Expression*)malloc(sizeof(Expression));
        assignment->type = AST_BINARY_EXPRESSION;
        assignment->value.binary.left = left;
        assignment->value.binary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        assignment->value.binary.right = parse_assignment_expression(parser);
        return assignment;
    }
    return left;
}

Expression* parse_equality_expression(Parser* parser) {
    Expression* left = parse_relational_expression(parser);
    while (parser_check_token(parser, TOKEN_OPERATOR) && (strcmp(parser->current_token->value, "==") == 0 || strcmp(parser->current_token->value, "!=") == 0)) {
        Expression* binary = (Expression*)malloc(sizeof(Expression));
        binary->type = AST_BINARY_EXPRESSION;
        binary->value.binary.left = left;
        binary->value.binary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        binary->value.binary.right = parse_relational_expression(parser);
        left = binary;
    }
    return left;
}

Expression* parse_relational_expression(Parser* parser) {
    Expression* left = parse_additive_expression(parser);
    while (parser_check_token(parser, TOKEN_OPERATOR) && (strcmp(parser->current_token->value, "<") == 0 || strcmp(parser->current_token->value, ">") == 0 || strcmp(parser->current_token->value, "<=") == 0 || strcmp(parser->current_token->value, ">=") == 0)) {
        Expression* binary = (Expression*)malloc(sizeof(Expression));
        binary->type = AST_BINARY_EXPRESSION;
        binary->value.binary.left = left;
        binary->value.binary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        binary->value.binary.right = parse_additive_expression(parser);
        left = binary;
    }
    return left;
}

Expression* parse_additive_expression(Parser* parser) {
    Expression* left = parse_multiplicative_expression(parser);
    while (parser_check_token(parser, TOKEN_OPERATOR) && (strcmp(parser->current_token->value, "+") == 0 || strcmp(parser->current_token->value, "-") == 0)) {
        Expression* binary = (Expression*)malloc(sizeof(Expression));
        binary->type = AST_BINARY_EXPRESSION;
        binary->value.binary.left = left;
        binary->value.binary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        binary->value.binary.right = parse_multiplicative_expression(parser);
        left = binary;
    }
    return left;
}

Expression* parse_multiplicative_expression(Parser* parser) {
    Expression* left = parse_unary_expression(parser);
    while (parser_check_token(parser, TOKEN_OPERATOR) && (strcmp(parser->current_token->value, "*") == 0 || strcmp(parser->current_token->value, "/") == 0 || strcmp(parser->current_token->value, "%") == 0)) {
        Expression* binary = (Expression*)malloc(sizeof(Expression));
        binary->type = AST_BINARY_EXPRESSION;
        binary->value.binary.left = left;
        binary->value.binary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        binary->value.binary.right = parse_unary_expression(parser);
        left = binary;
    }
    return left;
}

Expression* parse_unary_expression(Parser* parser) {
    if (parser_check_token(parser, TOKEN_OPERATOR) && (strcmp(parser->current_token->value, "-") == 0 || strcmp(parser->current_token->value, "!") == 0)) {
        Expression* unary = (Expression*)malloc(sizeof(Expression));
        unary->type = AST_UNARY_EXPRESSION;
        unary->value.unary.operator = strdup(parser->current_token->value);
        parser_consume(parser);
        unary->value.unary.operand = parse_unary_expression(parser);
        return unary;
    }
    return parse_primary_expression(parser);
}

Expression* parse_primary_expression(Parser* parser) {
    if (parser_check_token(parser, TOKEN_IDENTIFIER)) {
        Expression* identifier = (Expression*)malloc(sizeof(Expression));
        identifier->type = AST_IDENTIFIER_EXPRESSION;
        identifier->value.identifier = strdup(parser->current_token->value);
        parser_consume(parser);
        if (parser_check_token(parser, TOKEN_PUNCTUATOR) && strcmp(parser->current_token->value, "(") == 0) {
            return parse_call_expression(parser, identifier);
        }
        return identifier;
    } else if (parser_check_token(parser, TOKEN_INTEGER_LITERAL)) {
        Expression* literal = (Expression*)malloc(sizeof(Expression));
        literal->type = AST_INTEGER_LITERAL_EXPRESSION;
        literal->value.int_value = atoll(parser->current_token->value);
        parser_consume(parser);
        return literal;
    } else if (parser_check_token(parser, TOKEN_FLOAT_LITERAL)) {
        Expression* literal = (Expression*)malloc(sizeof(Expression));
        literal->type = AST_FLOAT_LITERAL_EXPRESSION;
        literal->value.float_value = atof(parser->current_token->value);
        parser_consume(parser);
        return literal;
    } else if (parser_check_token(parser, TOKEN_STRING_LITERAL)) {
        Expression* literal = (Expression*)malloc(sizeof(Expression));
        literal->type = AST_STRING_LITERAL_EXPRESSION;
        literal->value.string_value = strdup(parser->current_token->value);
        parser_consume(parser);
        return literal;
    } else if (parser_check_token(parser, TOKEN_PUNCTUATOR) && strcmp(parser->current_token->value, "(") == 0) {
        parser_consume(parser);
        Expression* expression = parse_expression(parser);
        parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
        return expression;
    }
    parser_error(parser, "Beklenmeyen token (birincil ifade bekleniyordu)");
    return NULL; // Erişilmemesi gereken yer
}

Expression* parse_call_expression(Parser* parser, Expression* callee) {
    Expression* call = (Expression*)malloc(sizeof(Expression));
    call->type = AST_CALL_EXPRESSION;
    call->value.call.function = callee;
    call->value.call.arguments = parse_argument_list(parser);
    return call;
}

ASTNode* parse_argument_list(Parser* parser) {
    parser_expect_token(parser, TOKEN_PUNCTUATOR, "(");
    ASTNode* head = NULL;
    ASTNode* current = NULL;

    if (!(parser_check_token(parser, TOKEN_PUNCTUATOR) && strcmp(parser->current_token->value, ")") == 0)) {
        Expression* arg = parse_expression(parser);
        head = (ASTNode*)arg;
        current = head;
        while (parser_check_token(parser, TOKEN_PUNCTUATOR) && strcmp(parser->current_token->value, ",") == 0) {
            parser_consume(parser);
            arg = parse_expression(parser);
            current->next = (ASTNode*)arg;
            current = current->next;
        }
    }
    parser_expect_token(parser, TOKEN_PUNCTUATOR, ")");
    return head;
}