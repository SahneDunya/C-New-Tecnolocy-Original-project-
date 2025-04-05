#ifndef CNT_MATCH_SEMANTICS_H
#define CNT_MATCH_SEMANTICS_H

#include "cnt_ast.h"
#include "cnt_symboltable.h"
#include <stdbool.h>

// Bir match ifadesinin semantik analizini gerçekleştiren fonksiyonun prototipi
bool analyze_match_statement(MatchStatementNode* match_stmt, SymbolTable* current_scope);

#endif