#ifndef MINIC_SEMANTIC_H
#define MINIC_SEMANTIC_H

#include "ast/ast.h"
#include "semantic/symbol_table.h"

typedef enum {
    SEM_OK,
    SEM_ERRORS,
    SEM_INTERNAL_ERROR
} SemanticResult;

typedef struct {
    SymbolTable symbol_table;
    int error_count;
    int had_internal_error;
} SemanticAnalyzer;

int semantic_analyzer_init(SemanticAnalyzer *analyzer);
SemanticResult semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *program);
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);

void semantic_report_error(SemanticAnalyzer *analyzer, int line, int column, const char *format, ...);
void semantic_report_undeclared(SemanticAnalyzer *analyzer, const char *name, int line, int column);
void semantic_report_internal_error(SemanticAnalyzer *analyzer);

#endif
