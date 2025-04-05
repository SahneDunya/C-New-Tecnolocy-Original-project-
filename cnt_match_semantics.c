#include "cnt_match_semantics.h"
#include "cnt_ast.h"
#include "cnt_expression.h"
#include "cnt_pattern.h"
#include "cnt_type.h"
#include "cnt_symboltable.h"
#include <stdio.h>

// Yardımcı fonksiyon: Bir desenin beklenen tip ile uyumlu olup olmadığını kontrol eder
static bool is_pattern_compatible_with_type(const Pattern* pattern, const Type* expected_type) {
    if (!pattern || !expected_type) {
        return false;
    }

    switch (pattern->type) {
        case PATTERN_LITERAL: {
            const LiteralPattern* literal_pattern = (const LiteralPattern*)pattern;
            if (literal_pattern->value && literal_pattern->value->evaluated_type) {
                return types_are_equal(literal_pattern->value->evaluated_type, expected_type);
            }
            return false;
        }
        case PATTERN_VARIABLE:
            // Değişken deseni her türle uyumludur (tip çıkarımı veya açık tip bildirimi gerekebilir)
            return true;
        case PATTERN_WILDCARD:
            // Joker karakter deseni her türle uyumludur
            return true;
        case PATTERN_TYPE_CHECK: {
            const TypeCheckPattern* type_pattern = (const TypeCheckPattern*)pattern;
            return types_are_equal(type_pattern->expected_type, expected_type);
        }
        default:
            return false;
    }
}

bool analyze_match_statement(MatchStatementNode* match_stmt, SymbolTable* current_scope) {
    if (!match_stmt) {
        return true; // Nothing to analyze
    }

    // 1. Eşleştirilecek ifadenin tipini analiz et
    if (match_stmt->expression) {
        // Burada ifadenin tipini analiz etmek için uygun fonksiyonu çağırmanız gerekir.
        // Örnek olarak, evaluate_expression fonksiyonunun olduğunu varsayalım.
        // if (!evaluate_expression(match_stmt->expression, current_scope)) {
        //     fprintf(stderr, "Hata: Match ifadesindeki ifadenin tipi belirlenemedi.\n");
        //     return false;
        // }
        // Şimdilik tip analizinin başarılı olduğunu varsayalım.
    } else {
        fprintf(stderr, "Hata: Match ifadesinde eşleştirilecek bir ifade yok.\n");
        return false;
    }

    Type* matched_expression_type = match_stmt->expression->evaluated_type;
    if (!matched_expression_type) {
        fprintf(stderr, "Hata: Match ifadesindeki ifadenin tipi belirlenemedi.\n");
        return false;
    }

    bool has_default_case = (match_stmt->default_body != NULL);
    bool has_at_least_one_case = (match_stmt->first_case != NULL);

    // 2. Her bir case bloğunu analiz et
    MatchCaseNode* current_case = match_stmt->first_case;
    while (current_case != NULL) {
        // 2.1. Desen ile eşleştirilecek ifadenin tipinin uyumlu olup olmadığını kontrol et
        if (current_case->pattern) {
            if (!is_pattern_compatible_with_type(current_case->pattern, matched_expression_type)) {
                fprintf(stderr, "Hata: Match ifadesindeki desen, eşleştirilecek ifadenin tipiyle uyumlu değil.\n");
                // İlgili desenin ve ifadenin konum bilgilerini de yazdırabilirsiniz.
                // Örneğin: fprintf(stderr, "  Desen tipi: %d, İfade tipi: %s\n", current_case->pattern->type, matched_expression_type->name);
                return false;
            }

            // 2.2. Değişken desenlerini kapsamda tanıt (eğer varsa)
            if (current_case->pattern->type == PATTERN_VARIABLE) {
                VariablePattern* var_pattern = (VariablePattern*)current_case->pattern;
                // Burada yeni bir sembol oluşturup geçerli kapsamda eklemeniz gerekebilir.
                // Sembolün tipi, matched_expression_type olmalıdır.
                // Örnek: symbol_table_add_symbol(current_scope, var_pattern->name, matched_expression_type);
                printf("Uyarı: Değişken deseni '%s' için tip çıkarımı henüz tam olarak desteklenmiyor.\n", var_pattern->name);
            }
        } else {
            fprintf(stderr, "Hata: Match ifadesindeki bir case bloğunda desen yok.\n");
            return false;
        }

        // 2.3. Case bloğunun içindeki ifadeleri analiz et
        if (current_case->body) {
            // Burada case bloğunun içindeki ifadeleri analiz etmek için uygun fonksiyonu çağırmanız gerekir.
            // Örneğin: if (!analyze_statement_list(current_case->body, current_scope)) { return false; }
            printf("Uyarı: Case bloğu içindeki ifadelerin analizi henüz tam olarak implemente edilmedi.\n");
        }

        current_case = current_case->next;
    }

    // 3. İsteğe bağlı: Tamlık kontrolü (tüm olası durumların ele alınıp alınmadığı)
    // Bu, eşleştirilecek ifadenin tipine ve desenlere bağlı olarak karmaşık bir analiz gerektirebilir.
    // Örneğin, bir enum üzerinde eşleştirme yapılıyorsa, tüm enum değerlerinin case'lerde yer alıp almadığı kontrol edilebilir.
    printf("Uyarı: Match ifadesi için tamlık kontrolü henüz implemente edilmedi.\n");

    return true;
}