#ifndef CNT_PARSER_H
#define CNT_PARSER_H

#include "cnt_lexer.h"
#include "cnt_ast.h"

// Parser Yapısı
typedef struct {
    Lexer* lexer;
    Token* current_token;
} Parser;

// Parser'ı Başlatma Fonksiyonu
Parser* parser_init(Lexer* lexer);

// Bir Sonraki Token'ı Tüketme Fonksiyonu
void parser_consume(Parser* parser);

// Geçerli Token'ın Türünü Kontrol Etme Fonksiyonu
bool parser_check_token(Parser* parser, TokenType type);

// Belirli Bir Token'ı Tüketme ve Türünü Kontrol Etme Fonksiyonu
void parser_expect_token(Parser* parser, TokenType type, const char* expected_value);

// Hata Raporlama Fonksiyonu
void parser_error(Parser* parser, const char* message);

// Üst Düzey Ayrıştırma Fonksiyonu
Program* parse_program(Parser* parser);

// Bildirimleri Ayrıştırma Fonksiyonları
Declaration* parse_declaration(Parser* parser);
Declaration* parse_function_declaration(Parser* parser);
Declaration* parse_variable_declaration(Parser* parser);
Declaration* parse_module_declaration(Parser* parser);
Declaration* parse_interface_declaration(Parser* parser);
Declaration* parse_implementation_block(Parser* parser);
TypeSpecifier* parse_type_specifier(Parser* parser);

// Deyimleri Ayrıştırma Fonksiyonları
Statement* parse_statement(Parser* parser);
Statement* parse_block_statement(Parser* parser);
Statement* parse_if_statement(Parser* parser);
Statement* parse_while_statement(Parser* parser);
Statement* parse_for_statement(Parser* parser);
Statement* parse_return_statement(Parser* parser);
Statement* parse_expression_statement(Parser* parser);

// İfadeleri Ayrıştırma Fonksiyonları
Expression* parse_expression(Parser* parser);
Expression* parse_assignment_expression(Parser* parser);
Expression* parse_equality_expression(Parser* parser);
Expression* parse_relational_expression(Parser* parser);
Expression* parse_additive_expression(Parser* parser);
Expression* parse_multiplicative_expression(Parser* parser);
Expression* parse_unary_expression(Parser* parser);
Expression* parse_primary_expression(Parser* parser);
Expression* parse_call_expression(Parser* parser, Expression* callee);
ASTNode* parse_argument_list(Parser* parser);

#endif