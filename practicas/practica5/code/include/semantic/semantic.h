#ifndef MINIC_SEMANTIC_H
#define MINIC_SEMANTIC_H

#include "ast/ast.h"
#include "semantic/symbol_table.h"

/* Resultado de semantic_analyze */
typedef enum {
    SEM_OK,              /* sin errores semanticos */
    SEM_ERRORS,          /* errores semanticos en el programa fuente */
    SEM_INTERNAL_ERROR   /* fallo interno (memoria); analisis detenido */
} SemanticResult;

typedef struct {
    SymbolTable symbol_table;
    int error_count;
    int had_internal_error;
} SemanticAnalyzer;

/* Devuelve 1 si tuvo exito, 0 si fallo (el analizador queda destruible). */
int semantic_analyzer_init(SemanticAnalyzer *analyzer);
SemanticResult semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *program);
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);

#endif
