#include <stdio.h>
#include <stdarg.h>

#include "semantic/semantic.h"
#include "semantic/type_checker.h"

static void visit_statement(SemanticAnalyzer *a, ASTNode *node);

/* ------------------------------------------------------------------ */
/* Diagnosticos                                                       */
/* ------------------------------------------------------------------ */
void
semantic_report_internal_error(SemanticAnalyzer *a)
{
    if (!a->had_internal_error)
        fprintf(stderr,
                "Error interno: no se pudo reservar memoria durante el analisis semantico.\n");
    a->had_internal_error = 1;
}

void
semantic_report_error(SemanticAnalyzer *a,
                      int line, int column,
                      const char *format, ...)
{
    va_list args;

    a->error_count++;
    fprintf(stderr, "Error semantico [%d:%d]:\n", line, column);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void
semantic_report_undeclared(SemanticAnalyzer *a,
                           const char *name,
                           int line, int col)
{
    a->error_count++;
    fprintf(stderr, "Error semantico [%d:%d]:\n"
                    "la variable '%s' no ha sido declarada.\n",
            line, col, name);
}

static void
report_redeclaration(SemanticAnalyzer *a,
                     const Symbol *prev,
                     int line, int col)
{
    a->error_count++;
    fprintf(stderr, "Error semantico [%d:%d]:\n"
                    "la variable '%s' ya fue declarada\n"
                    "en este ambito.\n"
                    "Declaracion anterior [%d:%d].\n",
            line, col, prev->name, prev->line, prev->column);
}


/* ------------------------------------------------------------------ */
/* Sentencias                                                         */
/* ------------------------------------------------------------------ */

static void
visit_variable_declaration(SemanticAnalyzer *a,
                           ASTNode *node)
{
    const char *name = node->data.variable_declaration.name;
    Symbol *prev = symbol_table_lookup_current(&a->symbol_table, name);

    if (prev != NULL) {
        report_redeclaration(a, prev, node->line, node->column);
    } else {
        Symbol *sym = symbol_create(name,
                                    type_from_declared(node->data.variable_declaration.declared_type),
                                    a->symbol_table.current_scope->level,
                                    node->line,
                                    node->column,
                                    node);
        SymbolTableResult r;
        if (sym == NULL) {
            semantic_report_internal_error(a);
            return;
        }
        r = symbol_table_insert(&a->symbol_table, sym);
        if (r != ST_OK) {
            symbol_destroy(sym);
            if (r == ST_INTERNAL_ERROR) {
                semantic_report_internal_error(a);
                return;
            }
        }
    }
    
    type_check_initialization(a, node);
}

static void
visit_assignment(SemanticAnalyzer *a,
                 ASTNode *node)
{
    Symbol *target = symbol_table_lookup(&a->symbol_table, node->data.assignment.name);

    if (target == NULL)
        semantic_report_undeclared(a, node->data.assignment.name, node->line, node->column);

    type_check_assignment(a, node, target);
}

static void
visit_print_statement(SemanticAnalyzer *a,
                      ASTNode *node)
{
    (void)type_check_expression(a, node->data.print_statement.expression);
}

static void
visit_if_statement(SemanticAnalyzer *a,
                   ASTNode *node)
{
    type_check_condition(a, node->data.if_statement.condition, CONDITION_IF);
    visit_statement(a, node->data.if_statement.then_branch);
    visit_statement(a, node->data.if_statement.else_branch);
}

static void
visit_while_statement(SemanticAnalyzer *a,
                      ASTNode *node)
{
    type_check_condition(a, node->data.while_statement.condition, CONDITION_WHILE);
    visit_statement(a, node->data.while_statement.body);
}

static void
visit_block(SemanticAnalyzer *a,
            ASTNode *node)
{
    size_t i;

    if (symbol_table_enter_scope(&a->symbol_table) != ST_OK) {
        semantic_report_internal_error(a);
        return;
    }

    for (i = 0; i < node->data.block.statements.count; i++) {
        if (a->had_internal_error)
            break;
        visit_statement(a, node->data.block.statements.items[i]);
    }

    if (symbol_table_exit_scope(&a->symbol_table) != ST_OK)
        semantic_report_internal_error(a);
}

static void
visit_statement(SemanticAnalyzer *a,
                ASTNode *node)
{
    if (node == NULL || a->had_internal_error)
        return;

    switch (node->type) {
    case AST_BLOCK:
        visit_block(a, node);
        break;
    case AST_VARIABLE_DECLARATION:
        visit_variable_declaration(a, node);
        break;
    case AST_ASSIGNMENT:
        visit_assignment(a, node);
        break;
    case AST_PRINT:
        visit_print_statement(a, node);
        break;
    case AST_IF:
        visit_if_statement(a, node);
        break;
    case AST_WHILE:
        visit_while_statement(a, node);
        break;
    default:
        (void)type_check_expression(a, node);
        break;
    }
}

static void
visit_program(SemanticAnalyzer *a,
              ASTNode *program)
{
    size_t i;
    for (i = 0; i < program->data.program.statements.count; i++) {
        if (a->had_internal_error)
            break;
        visit_statement(a, program->data.program.statements.items[i]);
    }
}


/* ------------------------------------------------------------------ */
/* API del analizador                                                 */
/* ------------------------------------------------------------------ */

int
semantic_analyzer_init(SemanticAnalyzer *a)
{
    if (a == NULL)
        return 0;
    
    a->error_count = 0;
    a->had_internal_error = 0;
    a->symbol_table.current_scope = NULL;
    a->symbol_table.scope_count = 0;
    if (symbol_table_init(&a->symbol_table) != ST_OK) {
        fprintf(stderr, "Error interno: no se pudo crear la tabla de simbolos.\n");
        symbol_table_destroy(&a->symbol_table);
        return 0;
    }
    return 1;
}

SemanticResult
semantic_analyze(SemanticAnalyzer *a,
                 ASTNode *program)
{
    if (a == NULL || program == NULL
        || program->type != AST_PROGRAM
        || a->symbol_table.current_scope == NULL)
        return SEM_INTERNAL_ERROR;
    
    visit_program(a, program);
    if (a->had_internal_error)
        return SEM_INTERNAL_ERROR;
    return a->error_count > 0 ? SEM_ERRORS : SEM_OK;
}

void
semantic_analyzer_destroy(SemanticAnalyzer *a)
{
    if (a != NULL)
        symbol_table_destroy(&a->symbol_table);
}
