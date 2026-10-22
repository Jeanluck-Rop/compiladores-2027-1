#include "parser/grammar.h"
#include "parser/parser_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef ASTNode *(*ParseFn)(Parser *);
 
typedef struct {
    TokenType token;
    BinaryOperator op;
} BinaryOpEntry;
 
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* Cada funcion corresponde a un no terminal de la gramatica de MiniC */
/* ---------- Sentencias ---------- */
static ASTNode *parse_statement(Parser *parser);
//static void parse_statement_list(Parser *parser);
static ASTNode *parse_declaration(Parser *parser);
static ASTNode *parse_assignment(Parser *parser);
static ASTNode *parse_print_statement(Parser *parser);
static ASTNode *parse_if_statement(Parser *parser);
static ASTNode *parse_while_statement(Parser *parser);
static ASTNode *parse_block(Parser *parser);

/* ---------- Expresiones ---------- */
static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_or_expression(Parser *parser);
static ASTNode *parse_and_expression(Parser *parser);
static ASTNode *parse_equality_expression(Parser *parser);
static ASTNode *parse_comparison_expression(Parser *parser);
static ASTNode *parse_additive_expression(Parser *parser);
static ASTNode *parse_multiplicative_expression(Parser *parser);
static ASTNode *parse_unary_expression(Parser *parser);
static ASTNode *parse_primary(Parser *parser);

/* Copiamos una cadena, el AST no puede apuntar a lexemas de tokens */
static char *
copy_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
 
    if (copy != NULL)
        memcpy(copy, s, n);
    return copy;
}
 
/* Fallo de memoria al construir un nodo, detenemos el analisis */
static ASTNode *
fail_memory(Parser *parser)
{
    if (!parser->internal_failure)
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
    parser->internal_failure = 1;
    return NULL;
}

static int
current_line(const Parser *parser)
{
    return (int)parser->current.line;
}

static int
current_column(const Parser *parser)
{
    return (int)parser->current.column;
}

/* Agregamos una sentencia a la lista; si falla, la liberamos */
static int
append_statement(Parser *parser,
                 ASTNodeList *list,
                 ASTNode *stmt)
{
    if (ast_node_list_append(list, stmt))
        return 1;
    ast_destroy(stmt);
    fail_memory(parser);
    return 0;
}

/* Elegimos la produccion de la sentencia segun el token de anticipacion y sincronizamos si hubo error */
static ASTNode*
parse_statement(Parser *parser)
{
    ASTNode *node = NULL;
    
    if (parser->internal_failure)
        return NULL;

    switch (parser->current.type) {
    case INT:
    case BOOL:
        node = parse_declaration(parser);
        break;
    case IDENTIFIER:
        node = parse_assignment(parser);
        break;
    case PRINT:
        node = parse_print_statement(parser);
        break;
    case IF:
        node = parse_if_statement(parser);
        break;
    case WHILE:
        node = parse_while_statement(parser);
        break;
    case LBRACE:
        node = parse_block(parser);
        break;
    default:
        parser_current_error(parser, "una sentencia");
        parser_advance(parser);
        break;
    }

    if (parser->panic_mode)
        parser_sync(parser);
    return node;
}

/* Reconocemos una declaracion: tipo, identificador, inicializacion opcional y ';' */
static ASTNode*
parse_declaration(Parser *parser)
{
    ASTDeclaredType type;

    if (parser->current.type == INT) {
        type = AST_TYPE_INT;
    } else {
        type = AST_TYPE_BOOL;
    }
    ASTNode *initializer = NULL;
    ASTNode *node;
    char *name;
    int line, column;

    parser_advance(parser);

    if (!parser_check(parser, IDENTIFIER)) {
        parser_current_error(parser, "un identificador");
        return NULL;
    }
 
    /* El token se destruira al avanzar: copiamos el nombre ahora */
    line = current_line(parser);
    column = current_column(parser);
    name = copy_string(parser->current.lexeme);
    if (name == NULL)
        return fail_memory(parser);
    parser_advance(parser);


    if (parser_match(parser, ASSIGN)) {
        initializer = parse_expression(parser);
        if (initializer == NULL) {
            free(name);
            return NULL;

        }
    }

    if (!parser_consume(parser, SEMICOLON, "';'")) {
        ast_destroy(initializer);
        free(name);
        return NULL;
    }
    
    node = ast_create_variable_declaration(type, name, initializer, line, column);

    free(name);
    if (node == NULL) {
        ast_destroy(initializer);
        return fail_memory(parser);
    }
    return node;
}

/* Reconocemos una asignacion: identificador, '=', expresion y ';' */
static ASTNode*
parse_assignment(Parser *parser)
{
    ASTNode *value;
    ASTNode *node;
    char *name;
    int line = current_line(parser);
    int column = current_column(parser);

    name = copy_string(parser->current.lexeme);
    if (name == NULL)
        return fail_memory(parser);
    parser_advance(parser);

    if (!parser_consume(parser, ASSIGN, "'='")) {
        free(name);
        return NULL;
    }
    
    value = parse_expression(parser);
    if (value == NULL) {
        free(name);
        return NULL;
    }

    if (!parser_consume(parser, SEMICOLON, "';'")) {
        ast_destroy(value);
        free(name);
        return NULL;
    }

    node = ast_create_assignment(name, value, line, column);
    free(name);
    if (node == NULL) {
        ast_destroy(value);
        return fail_memory(parser);
    }
    return node;
}

/* Reconocemos una impresion: print, expresion entre parentesis y ';' */
static ASTNode*
parse_print_statement(Parser *parser)
{
    ASTNode *expr = NULL;
    ASTNode *node;
    int line = current_line(parser);
    int column = current_column(parser);

    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'('"))
        return NULL;

    expr = parse_expression(parser);
    
    if (expr == NULL)
        return NULL;

    if (!parser_consume(parser, RPAREN, "')'")
        || !parser_consume(parser, SEMICOLON, "';'")) {
        ast_destroy(expr);
        return NULL;
    }

    node = ast_create_print(expr, line, column);
    
    if (node == NULL) {
        ast_destroy(expr);
        return fail_memory(parser);
    }
    return node;
}

/* Reconocemos un if con else opcional, que se asocia con el if mas cercano */
static ASTNode*
parse_if_statement(Parser *parser)
{
    ASTNode *condition;
    ASTNode *then_branch;
    ASTNode *else_branch = NULL;
    ASTNode *node;
    int has_else = 0;
    int line = current_line(parser);
    int column = current_column(parser);

    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'(' despues de 'if'"))
        return NULL;

    condition = parse_expression(parser);
    if (condition == NULL)
        return NULL;

    if (!parser_consume(parser, RPAREN, "')'")) {
        ast_destroy(condition);
        return NULL;
    }

    then_branch = parse_statement(parser);

    if (parser_match(parser, ELSE)) {
        has_else = 1;
        else_branch = parse_statement(parser);
    }
 
    if (then_branch == NULL || (has_else && else_branch == NULL)) {
        ast_destroy(condition);
        ast_destroy(then_branch);
        ast_destroy(else_branch);
        return NULL;
    }
 
    node = ast_create_if(condition, then_branch, else_branch, line, column);
    if (node == NULL) {
        ast_destroy(condition);
        ast_destroy(then_branch);
        ast_destroy(else_branch);
        return fail_memory(parser);
    }
    return node;

}

/* Reconocemos un ciclo while: condicion entre parentesis y una sentencia como cuerpo */
static ASTNode*
parse_while_statement(Parser *parser)
{
    ASTNode *condition;
    ASTNode *body;
    ASTNode *node;
    int line = current_line(parser);
    int column = current_column(parser);

    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'(' despues de 'while'"))
        return NULL;

    condition = parse_expression(parser);
    if (condition == NULL)
        return NULL;

    if (!parser_consume(parser, RPAREN, "')'"))  {
        ast_destroy(condition);
        return NULL;
    }

    body = parse_statement(parser);
    if (body == NULL) {
        ast_destroy(condition);
        return NULL;
    }

    node = ast_create_while(condition, body, line, column);
    if (node == NULL) {
        ast_destroy(condition);
        ast_destroy(body);
        return fail_memory(parser);
    }
    return node;
}

/* Reconocemos un bloque: sentencias entre llaves hasta encontrar '}' */
static ASTNode*
parse_block(Parser *parser)
{
    ASTNodeList list;
    ASTNode *node;
    int failed = 0;
    int line = current_line(parser);
    int column = current_column(parser);

    ast_node_list_init(&list);
    parser_advance(parser);

    while (!parser_check(parser, RBRACE)
           && !parser_end(parser)) {
        ASTNode *stmt = parse_statement(parser);
 
        if (stmt == NULL) {
            failed = 1;
            continue;
        }
        if (!append_statement(parser, &list, stmt)) {
            failed = 1;
            break;
        }
    }

    if (!parser_consume(parser, RBRACE, "'}'"))
        failed = 1;
 
    if (failed) {
        ast_node_list_destroy(&list);
        return NULL;
    }
 
    node = ast_create_block(list, line, column);
    if (node == NULL) {
        ast_node_list_destroy(&list);
        return fail_memory(parser);
    }
    return node;
}

/*
 * Nivel generico de operadores binarios asociativos por la izquierda:
 *   level ::= next ( (op1 | op2 | ...) next )*
 * El nodo se posiciona en el token del operador.
 */
static ASTNode*
parse_binary_level(Parser *parser,
                   ParseFn next,
                   const BinaryOpEntry *ops,
                   size_t count)
{
    ASTNode *left = next(parser);
 
    while (left != NULL
           && !parser->panic_mode
           && !parser->internal_failure) {
        const BinaryOpEntry *found = NULL;
        ASTNode *right;
        ASTNode *node;
        size_t i;
        int line, column;
 
        for (i = 0; i < count; i++) {
            if (parser_check(parser, ops[i].token)) {
                found = &ops[i];
                break;
            }
        }
        if (found == NULL)
            break;
 
        line = current_line(parser);
        column = current_column(parser);
        parser_advance(parser);
 
        right = next(parser);
        if (right == NULL) {
            ast_destroy(left);
            return NULL;
        }
 
        node = ast_create_binary(found->op, left, right, line, column);
        if (node == NULL) {
            ast_destroy(left);
            ast_destroy(right);
            return fail_memory(parser);
        }
        left = node;
    }
 
    return left;
}

/* Punto de entrada de las expresiones, empezando por el nivel de menor precedencia */
static ASTNode*
parse_expression(Parser *parser)
{
    return parse_or_expression(parser);
}

/* Reconocemos disyunciones '||', asociativas por la izquierda */
static ASTNode*
parse_or_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = { { OR, OP_OR } };
    return parse_binary_level(parser, parse_and_expression,
                              ops, COUNT(ops));

}

/* Reconocemos conjunciones '&&', asociativas por la izquierda */
static ASTNode*
parse_and_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = { { AND, OP_AND } };
    return parse_binary_level(parser, parse_equality_expression,
                              ops, COUNT(ops));

}

/* Reconocemos comparaciones de igualdad '==' y '!=' */
static ASTNode*
parse_equality_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = {
        { EQUAL, OP_EQUAL },
        { NOT_EQUAL, OP_NOT_EQUAL }
    };
    return parse_binary_level(parser, parse_comparison_expression,
                              ops, COUNT(ops));

}

/* Reconocemos operadores relacionales '<', '<=', '>' y '>=' */
static ASTNode*
parse_comparison_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = {
        { LESS, OP_LESS },
        { LESS_EQUAL, OP_LESS_EQUAL },
        { GREATER, OP_GREATER },
        { GREATER_EQUAL, OP_GREATER_EQUAL }
    };
    return parse_binary_level(parser, parse_additive_expression,
                              ops, COUNT(ops));

}

/* Reconocemos sumas y restas '+' y '-' */
static ASTNode*
parse_additive_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = {
        { PLUS, OP_ADD },
        { MINUS, OP_SUBTRACT }
    };
    return parse_binary_level(parser, parse_multiplicative_expression,
                              ops, COUNT(ops));

}

/* Reconocemos multiplicaciones y divisiones '*' y '/' */
static ASTNode*
parse_multiplicative_expression(Parser *parser)
{
    static const BinaryOpEntry ops[] = {
        { STAR, OP_MULTIPLY },
        { SLASH, OP_DIVIDE }
    };
    return parse_binary_level(parser, parse_unary_expression,
                              ops, COUNT(ops));

}

/* Reconocemos la negacion unaria '-', asociativa por la derecha mediante recursion */
static ASTNode*
parse_unary_expression(Parser *parser)
{
    ASTNode *operand;
    ASTNode *node;
    int line, column;

    if (!parser_check(parser, MINUS))
        return parse_primary(parser);
 
    line = current_line(parser);
    column = current_column(parser);
    parser_advance(parser);
 
    operand = parse_unary_expression(parser);
    if (operand == NULL)
        return NULL;
 
    node = ast_create_unary(OP_NEGATE, operand, line, column);
    if (node == NULL) {
        ast_destroy(operand);
        return fail_memory(parser);
    }
    return node;
}

/* Reconocemos literales, identificadores o una expresion entre parentesis */
static ASTNode*
parse_primary(Parser *parser)
{
    ASTNode *node = NULL;
    ASTNode *inner;
    int line, column;

    if (parser->internal_failure)
        return NULL;

    line = current_line(parser);
    column = current_column(parser);


    switch (parser->current.type) {
    case INTEGER:
        node = ast_create_integer(parser->current.lexeme, line, column);
        break;
    case TRUE:
        node = ast_create_boolean(1, line, column);
        break;
    case FALSE:
        node = ast_create_boolean(0, line, column);
        break;
    case IDENTIFIER:
        node = ast_create_identifier(parser->current.lexeme, line, column);
        break;
    case LPAREN:
        /* Los parentesis no generan nodo: se devuelve la expresion interna */
        parser_advance(parser);
        inner = parse_expression(parser);
        if (inner == NULL)
            return NULL;
        if (!parser_consume(parser, RPAREN, "')'")) {
            ast_destroy(inner);
            return NULL;
        }
        return inner;


    default:
        parser_current_error(parser, "una expresion");
        return NULL;
    }

    if (node == NULL)
        return fail_memory(parser);
 
    parser_advance(parser);
    return node;

}

/*
 * Reconocemos el programa completo y verificamos que termine en TOKEN_EOF
 * program ::= statement* TOKEN_EOF
 * Devuelve la raiz, o NULL si hubo cualquier error lexico, sintactico
 * o interno (en ese caso no queda ningun nodo sin liberar).
 */
ASTNode*
parse_program(Parser *parser)
{
    ASTNodeList list;
    ASTNode *root;
    int failed = 0;
    int line, column;
 
    ast_node_list_init(&list);
 
    while (!parser_end(parser)) {
        ASTNode *stmt = parse_statement(parser);
 
        if (stmt == NULL) {
            failed = 1;
            continue;
        }
        if (!append_statement(parser, &list, stmt)) {
            failed = 1;
            break;
        }
    }
 
    if (!parser->internal_failure && !parser_check(parser, TOKEN_EOF))
        parser_current_error(parser, "el fin del archivo");
 
    if (failed || parser->had_error || parser->internal_failure) {
        ast_node_list_destroy(&list);
        return NULL;
    }
 
    /* Posicion: primera sentencia o, si no hay, TOKEN_EOF */
    if (list.count > 0) {
        line = list.items[0]->line;
        column = list.items[0]->column;
    } else {
        line = current_line(parser);
        column = current_column(parser);
    }
 
    root = ast_create_program(list, line, column);
    if (root == NULL) {
        ast_node_list_destroy(&list);
        return fail_memory(parser);
    }
    return root;
}
