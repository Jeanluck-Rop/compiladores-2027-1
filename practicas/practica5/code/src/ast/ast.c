#include "ast/ast.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Creamos una copia independiente de una cadena */
static char*
copy_string(const char *s)
{
    size_t n;
    char *copy;

    if (s == NULL)
        return NULL;
    n = strlen(s) + 1;
    copy = malloc(n);
    if (copy == NULL)
        return NULL;
    memcpy(copy, s, n);
    return copy;
}

/* Reservamos memoria de un nodo del AST */
static ASTNode*
alloc_node(ASTNodeType type,
           int line,
           int column)
{
    ASTNode *node = calloc(1, sizeof(*node));

    if (node == NULL)
        return NULL;
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

/* Inicializamos un nodo del AST */
void
ast_node_list_init(ASTNodeList *list)
{
    if (list == NULL)
        return;

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Agregamos un nodo al final de la lista */
int
ast_node_list_append(ASTNodeList *list,
                     ASTNode *node)
{
    if (list == NULL || node == NULL)
        return 0;

    if (list->count == list->capacity) {
        size_t new_capacity = list->capacity ? list->capacity * 2 : 8;
        ASTNode **items;

        if (new_capacity > SIZE_MAX / sizeof(*items))
            return 0;
        items = realloc(list->items, new_capacity * sizeof(*items));
        if (items == NULL)
            return 0;
        list->items = items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = node;
    return 1;
}

/*
 * Libera todos los nodos almacenados en la lista y posteriormente
 * libera el arreglo que contiene sus apuntadores
 */
void
ast_node_list_destroy(ASTNodeList *list)
{
    size_t i;

    if (list == NULL)
        return;
    for (i = 0; i < list->count; i++)
        ast_destroy(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/*
 * Constructores
 * Exito: el nodo adquiere los hijos / la lista
 * Fallo: los hijos / la lista siguen siendo de quien llama
 */
ASTNode *
ast_create_program(ASTNodeList statements,
                   int line,
                   int column)
{
    ASTNode *node = alloc_node(AST_PROGRAM, line, column);

    if (node == NULL)
        return NULL;
    node->data.program.statements = statements;
    return node;
}

/* Crea un nodo que representa un bloque de sentencias */
ASTNode*
ast_create_block(ASTNodeList statements,
                 int line,
                 int column)
{
    ASTNode *node = alloc_node(AST_BLOCK, line, column);

    if (node == NULL)
        return NULL;
    node->data.block.statements = statements;
    return node;
}

/* Creamos un nodo que representa una declaracion de variable */
ASTNode*
ast_create_variable_declaration(ASTDeclaredType declared_type,
                                const char *name,
                                ASTNode *initializer,
                                int line,
                                int column)
{
    ASTNode *node;
    char *copy = copy_string(name);

    if (copy == NULL)
        return NULL;
    node = alloc_node(AST_VARIABLE_DECLARATION, line, column);
    if (node == NULL) {
        free(copy);
        return NULL;
    }
    node->data.variable_declaration.declared_type = declared_type;
    node->data.variable_declaration.name = copy;
    node->data.variable_declaration.initializer = initializer;
    return node;
}

/* Creamos un nodo que representa una asignacion */
ASTNode*
ast_create_assignment(const char *name,
                      ASTNode *value,
                      int line,
                      int column)
{
    ASTNode *node;
    char *copy = copy_string(name);

    if (copy == NULL)
        return NULL;
    node = alloc_node(AST_ASSIGNMENT, line, column);
    if (node == NULL) {
        free(copy);
        return NULL;
    }
    node->data.assignment.name = copy;
    node->data.assignment.value = value;
    return node;
}

/* Creamos un nodo que representa un print */
ASTNode*
ast_create_print(ASTNode *expression, int line, int column)
{
    ASTNode *node = alloc_node(AST_PRINT, line, column);

    if (node == NULL)
        return NULL;
    node->data.print_statement.expression = expression;
    return node;
}

/* Creamos un nodo que representa una condicional */
ASTNode*
ast_create_if(ASTNode *condition,
              ASTNode *then_branch,
              ASTNode *else_branch,
              int line,
              int column)
{
    ASTNode *node = alloc_node(AST_IF, line, column);

    if (node == NULL)
        return NULL;
    node->data.if_statement.condition = condition;
    node->data.if_statement.then_branch = then_branch;
    node->data.if_statement.else_branch = else_branch;
    return node;
}

/* Creamos un nodo que representa un while */
ASTNode *
ast_create_while(ASTNode *condition, ASTNode *body, int line, int column)
{
    ASTNode *node = alloc_node(AST_WHILE, line, column);

    if (node == NULL)
        return NULL;
    node->data.while_statement.condition = condition;
    node->data.while_statement.body = body;
    return node;
}

/*
 * Creamos una expresion binaria y almacenamos el operador
 * junto con sus dos operandos izq y der.
 */
ASTNode*
ast_create_binary(BinaryOperator operator,
                  ASTNode *left,
                  ASTNode *right,
                  int line,
                  int column)
{
    ASTNode *node = alloc_node(AST_BINARY_EXPRESSION, line, column);

    if (node == NULL)
        return NULL;
    node->data.binary.operator = operator;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

/* Creamos una expresion unaria */
ASTNode *
ast_create_unary(UnaryOperator operator,
                 ASTNode *operand,
                 int line,
                 int column)
{
    ASTNode *node = alloc_node(AST_UNARY_EXPRESSION, line, column);

    if (node == NULL)
        return NULL;
    node->data.unary.operator = operator;
    node->data.unary.operand = operand;
    return node;
}

/* Creamos un nodo que representa un identificador */
ASTNode*
ast_create_identifier(const char *name,
                      int line,
                      int column)
{
    ASTNode *node;
    char *copy = copy_string(name);

    if (copy == NULL)
        return NULL;
    node = alloc_node(AST_IDENTIFIER, line, column);
    if (node == NULL) {
        free(copy);
        return NULL;
    }
    node->data.identifier.name = copy;
    return node;
}

/* Crea un nodo que representa un literal entero */
ASTNode*
ast_create_integer(const char *lexeme,
                   int line,
                   int column)
{
    ASTNode *node;
    char *copy = copy_string(lexeme);

    if (copy == NULL)
        return NULL;
    node = alloc_node(AST_INTEGER_LITERAL, line, column);
    if (node == NULL) {
        free(copy);
        return NULL;
    }
    node->data.integer_literal.lexeme = copy;
    return node;
}

/* Creamos un nodo que representa un literal booleano */
ASTNode*
ast_create_boolean(int value,
                   int line,
                   int column)
{
    ASTNode *node = alloc_node(AST_BOOLEAN_LITERAL, line, column);
    if (node == NULL)
        return NULL;
    node->data.boolean_literal.value = value ? 1 : 0;
    return node;
}

/* Imprimimos en formato canonico */
static void
indent(int depth)
{
    int i;
    for (i = 0; i < depth * 2; i++)
        putchar(' ');
}

/* */
static void
print_label(int depth,
            const char *label)
{
    indent(depth);
    puts(label);
}

/* */
static const char*
binary_operator_name(BinaryOperator op)
{
    switch (op) {
    case OP_ADD:
        return "Add";
    case OP_SUBTRACT:
        return "Subtract";
    case OP_MULTIPLY:
        return "Multiply";
    case OP_DIVIDE:
        return "Divide";
    case OP_LESS:
        return "Less";
    case OP_LESS_EQUAL:
        return "LessEqual";
    case OP_GREATER:
        return "Greater";
    case OP_GREATER_EQUAL:
        return "GreaterEqual";
    case OP_EQUAL:
        return "Equal";
    case OP_NOT_EQUAL:
        return "NotEqual";
    case OP_AND:
        return "And";
    case OP_OR:
        return "Or";
    }
    return "?";
}

/* */
static void print_node(const ASTNode *node, int depth);

/* */
static void
print_list(const ASTNodeList *list,
           int depth)
{
    size_t i;

    for (i = 0; i < list->count; i++)
        print_node(list->items[i], depth);
}

/* */
static void
print_node(const ASTNode *node,
           int depth)
{
    if (node == NULL)
        return;

    switch (node->type) {
    case AST_PROGRAM:
        print_label(depth, "Program");
        print_list(&node->data.program.statements, depth + 1);
        break;

    case AST_BLOCK:
        print_label(depth, "Block");
        print_list(&node->data.block.statements, depth + 1);
        break;

    case AST_VARIABLE_DECLARATION:
        indent(depth);
        printf("VariableDeclaration(%s, %s)\n",
               node->data.variable_declaration.declared_type == AST_TYPE_INT
                   ? "int" : "bool",
               node->data.variable_declaration.name);
        print_node(node->data.variable_declaration.initializer, depth + 1);
        break;

    case AST_ASSIGNMENT:
        indent(depth);
        printf("Assignment(%s)\n", node->data.assignment.name);
        print_node(node->data.assignment.value, depth + 1);
        break;

    case AST_PRINT:
        print_label(depth, "Print");
        print_node(node->data.print_statement.expression, depth + 1);
        break;

    case AST_IF:
        print_label(depth, "If");
        print_label(depth + 1, "Condition");
        print_node(node->data.if_statement.condition, depth + 2);
        print_label(depth + 1, "Then");
        print_node(node->data.if_statement.then_branch, depth + 2);
        if (node->data.if_statement.else_branch != NULL) {
            print_label(depth + 1, "Else");
            print_node(node->data.if_statement.else_branch, depth + 2);
        }
        break;

    case AST_WHILE:
        print_label(depth, "While");
        print_label(depth + 1, "Condition");
        print_node(node->data.while_statement.condition, depth + 2);
        print_label(depth + 1, "Body");
        print_node(node->data.while_statement.body, depth + 2);
        break;

    case AST_BINARY_EXPRESSION:
        print_label(depth, binary_operator_name(node->data.binary.operator));
        print_node(node->data.binary.left, depth + 1);
        print_node(node->data.binary.right, depth + 1);
        break;

    case AST_UNARY_EXPRESSION:
        print_label(depth, "Negate");
        print_node(node->data.unary.operand, depth + 1);
        break;

    case AST_IDENTIFIER:
        indent(depth);
        printf("Identifier(%s)\n", node->data.identifier.name);
        break;

    case AST_INTEGER_LITERAL:
        indent(depth);
        printf("Integer(%s)\n", node->data.integer_literal.lexeme);
        break;

    case AST_BOOLEAN_LITERAL:
        print_label(depth, node->data.boolean_literal.value
                               ? "Boolean(true)" : "Boolean(false)");
        break;
    }
}

void
ast_print(const ASTNode *node)
{
    print_node(node, 0);
}

/* Liberacion */
void
ast_destroy(ASTNode *node)
{
    if (node == NULL)
        return;

    switch (node->type) {
    case AST_PROGRAM:
        ast_node_list_destroy(&node->data.program.statements);
        break;
    case AST_BLOCK:
        ast_node_list_destroy(&node->data.block.statements);
        break;
    case AST_VARIABLE_DECLARATION:
        free(node->data.variable_declaration.name);
        ast_destroy(node->data.variable_declaration.initializer);
        break;
    case AST_ASSIGNMENT:
        free(node->data.assignment.name);
        ast_destroy(node->data.assignment.value);
        break;
    case AST_PRINT:
        ast_destroy(node->data.print_statement.expression);
        break;
    case AST_IF:
        ast_destroy(node->data.if_statement.condition);
        ast_destroy(node->data.if_statement.then_branch);
        ast_destroy(node->data.if_statement.else_branch);
        break;
    case AST_WHILE:
        ast_destroy(node->data.while_statement.condition);
        ast_destroy(node->data.while_statement.body);
        break;
    case AST_BINARY_EXPRESSION:
        ast_destroy(node->data.binary.left);
        ast_destroy(node->data.binary.right);
        break;
    case AST_UNARY_EXPRESSION:
        ast_destroy(node->data.unary.operand);
        break;
    case AST_IDENTIFIER:
        free(node->data.identifier.name);
        break;
    case AST_INTEGER_LITERAL:
        free(node->data.integer_literal.lexeme);
        break;
    case AST_BOOLEAN_LITERAL:
        break;
    }

    free(node);
}
