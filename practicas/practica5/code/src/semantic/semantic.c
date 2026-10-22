#include "semantic/semantic.h"
#include <stdio.h>

static void visit_statement(SemanticAnalyzer *a, ASTNode *node);
static void visit_expression(SemanticAnalyzer *a, ASTNode *node);

static void
report_internal_error(SemanticAnalyzer *a)
{
    if (!a->had_internal_error) {
        fprintf(stderr, "Error interno: no se pudo reservar memoria "
                        "durante el analisis semantico.\n");
    }
    a->had_internal_error = 1;
}

static void
report_undeclared(SemanticAnalyzer *a, const char *name, int line, int col)
{
    a->error_count++;
    fprintf(stderr, "Error semantico [%d:%d]:\n"
                    "la variable '%s' no ha sido declarada.\n",
            line, col, name);
}

static void
report_redeclaration(SemanticAnalyzer *a, const Symbol *prev,
                     int line, int col)
{
    a->error_count++;
    fprintf(stderr, "Error semantico [%d:%d]:\n"
                    "la variable '%s' ya fue declarada\n"
                    "en este ambito.\n"
                    "Declaracion anterior [%d:%d].\n",
            line, col, prev->name, prev->line, prev->column);
}

static Type
type_from_ast(ASTDeclaredType t)
{
    return t == AST_TYPE_BOOL ? TYPE_BOOL : TYPE_INT;
}

static void
visit_identifier_use(SemanticAnalyzer *a, const char *name, int line, int col)
{
    if (symbol_table_lookup(&a->symbol_table, name) == NULL) {
        report_undeclared(a, name, line, col);
    }
}

static void
visit_expression(SemanticAnalyzer *a, ASTNode *node)
{
    if (node == NULL || a->had_internal_error) {
        return;
    }
    switch (node->type) {
    case AST_IDENTIFIER:
        visit_identifier_use(a, node->data.identifier.name,
                             node->line, node->column);
        break;
    case AST_BINARY_EXPRESSION:
        visit_expression(a, node->data.binary.left);
        visit_expression(a, node->data.binary.right);
        break;
    case AST_UNARY_EXPRESSION:
        visit_expression(a, node->data.unary.operand);
        break;
    default:               /* literales: nada que buscar */
        break;
    }
}

static void
visit_variable_declaration(SemanticAnalyzer *a, ASTNode *node)
{
    const char *name = node->data.variable_declaration.name;
    Symbol *prev = symbol_table_lookup_current(&a->symbol_table, name);

    if (prev != NULL) {
        /* No se sobrescribe el simbolo original. */
        report_redeclaration(a, prev, node->line, node->column);
    } else {
        Symbol *sym = symbol_create(
            name, type_from_ast(node->data.variable_declaration.declared_type),
            a->symbol_table.current_scope->level,
            node->line, node->column, node);
        SymbolTableResult r;

        if (sym == NULL) {
            report_internal_error(a);
            return;
        }
        r = symbol_table_insert(&a->symbol_table, sym);
        if (r != ST_OK) {          /* la tabla no adquirio el simbolo */
            symbol_destroy(sym);
            if (r == ST_INTERNAL_ERROR) {
                report_internal_error(a);
                return;
            }
        }
    }
    /* El simbolo ya es visible: int x = x; es valido. */
    visit_expression(a, node->data.variable_declaration.initializer);
}

static void
visit_assignment(SemanticAnalyzer *a, ASTNode *node)
{
    visit_identifier_use(a, node->data.assignment.name,
                         node->line, node->column);
    visit_expression(a, node->data.assignment.value);
}

static void
visit_print_statement(SemanticAnalyzer *a, ASTNode *node)
{
    visit_expression(a, node->data.print_statement.expression);
}

static void
visit_if_statement(SemanticAnalyzer *a, ASTNode *node)
{
    visit_expression(a, node->data.if_statement.condition);
    visit_statement(a, node->data.if_statement.then_branch);
    visit_statement(a, node->data.if_statement.else_branch);
}

static void
visit_while_statement(SemanticAnalyzer *a, ASTNode *node)
{
    visit_expression(a, node->data.while_statement.condition);
    visit_statement(a, node->data.while_statement.body);
}

static void
visit_block(SemanticAnalyzer *a, ASTNode *node)
{
    size_t i;

    if (symbol_table_enter_scope(&a->symbol_table) != ST_OK) {
        report_internal_error(a);
        return;                    /* no se entro: no se sale */
    }
    for (i = 0; i < node->data.block.statements.count; i++) {
        if (a->had_internal_error) {
            break;
        }
        visit_statement(a, node->data.block.statements.items[i]);
    }
    if (symbol_table_exit_scope(&a->symbol_table) != ST_OK) {
        report_internal_error(a);
    }
}

static void
visit_statement(SemanticAnalyzer *a, ASTNode *node)
{
    if (node == NULL || a->had_internal_error) {
        return;
    }
    switch (node->type) {
    case AST_BLOCK:                 visit_block(a, node); break;
    case AST_VARIABLE_DECLARATION:  visit_variable_declaration(a, node); break;
    case AST_ASSIGNMENT:            visit_assignment(a, node); break;
    case AST_PRINT:                 visit_print_statement(a, node); break;
    case AST_IF:                    visit_if_statement(a, node); break;
    case AST_WHILE:                 visit_while_statement(a, node); break;
    default:                        visit_expression(a, node); break;
    }
}

static void
visit_program(SemanticAnalyzer *a, ASTNode *program)
{
    size_t i;
    for (i = 0; i < program->data.program.statements.count; i++) {
        if (a->had_internal_error) {
            break;
        }
        visit_statement(a, program->data.program.statements.items[i]);
    }
}

int
semantic_analyzer_init(SemanticAnalyzer *a)
{
    if (a == NULL) {
        return 0;
    }
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
semantic_analyze(SemanticAnalyzer *a, ASTNode *program)
{
    if (a == NULL || program == NULL || program->type != AST_PROGRAM
        || a->symbol_table.current_scope == NULL) {
        return SEM_INTERNAL_ERROR;
    }
    visit_program(a, program);
    if (a->had_internal_error) {
        return SEM_INTERNAL_ERROR;
    }
    return a->error_count > 0 ? SEM_ERRORS : SEM_OK;
}

void
semantic_analyzer_destroy(SemanticAnalyzer *a)
{
    if (a != NULL) {
        symbol_table_destroy(&a->symbol_table);
    }
}
