#include "ast/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Funciones auxiliares
static char *ast_strdup(const char *text) {
    if (text == NULL){
        return NULL;
    }

    size_t length = strlen(text);

    char *copy = malloc(length + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length + 1);

    return copy;
}

static ASTNode *ast_alloc(ASTNodeType type, int line, int column) {
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        return NULL;
    }

    node->type = type;
    node->line = line;
    node->column = column;

    return node;
}

//Listas de nodos
void ast_node_list_init(ASTNodeList *list) {
    if (list == NULL) {
        return;
    }

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

//Modificado
int ast_node_list_append(ASTNodeList *list, ASTNode *node) {
    if (list == NULL || node == NULL) {
        return 0;
    }

    if (list->count == list->capacity) {
        size_t new_capacity;

        if (list->capacity == 0) {
            new_capacity = 4;
        } else {
            new_capacity = list->capacity * 2;
        }

        ASTNode **new_items = realloc(list->items, new_capacity * sizeof(ASTNode *));

        if (new_items == NULL) {
            return 0;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }
    
    list->items[list->count] = node;
    list->count++;
    
    return 1;
}

//Modificado
void ast_node_list_destroy(ASTNodeList *list) {
    if (list == NULL) {
        return;
    }

    for (size_t i = 0; i < list->count; i++) {
        ast_destroy(list->items[i]);
    }

    free(list->items);

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

//Programa
//Modificado
ASTNode *ast_create_program(
    ASTNodeList statements,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(AST_PROGRAM, line, column);

    if (node == NULL) {
        return NULL;
    }

    node->data.program.statements = statements;

    return node;
}

//Bloque
// Modificado
ASTNode *ast_create_block(
    ASTNodeList statements,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(AST_BLOCK, line, column);

    if (node == NULL) {
        return NULL;
    }

    node->data.block.statements = statements;

    return node;
}

//Declaración de variable
// Modificado
ASTNode *ast_create_variable_declaration(
    ASTDeclaredType declared_type,
    const char *name,
    ASTNode *initializer,
    int line,
    int column
) {
    if (name == NULL) {
        return NULL;
    }

    ASTNode *node = ast_alloc(
        AST_VARIABLE_DECLARATION,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.variable_declaration.name = ast_strdup(name);

    if (node->data.variable_declaration.name == NULL) {
        free(node);
        return NULL;
    }

    node->data.variable_declaration.declared_type = declared_type;
    node->data.variable_declaration.initializer = initializer;

    return node;
}

//Asignación
// Modificado
ASTNode *ast_create_assignment(
    const char *name,
    ASTNode *value,
    int line,
    int column
) {
    if (name == NULL) {
        return NULL;
    }

    ASTNode *node = ast_alloc(
        AST_ASSIGNMENT,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.assignment.name = ast_strdup(name);

    if (node->data.assignment.name == NULL) {
        free(node);
        return NULL;
    }

    node->data.assignment.value = value;

    return node;
}

// Modificado
ASTNode *ast_create_print(
    ASTNode *expression,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(
        AST_PRINT,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.print_statement.expression = expression;

    return node;
}

// Modificado
ASTNode *ast_create_if(
    ASTNode *condition,
    ASTNode *then_branch,
    ASTNode *else_branch,
    int line,
    int column
) {
   ASTNode *node = ast_alloc(
        AST_IF,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.if_statement.condition = condition;
    node->data.if_statement.then_branch = then_branch;
    node->data.if_statement.else_branch = else_branch;

    return node;
}

//Modificado
ASTNode *ast_create_while(
    ASTNode *condition,
    ASTNode *body,
    int line,
    int column
) {
   ASTNode *node = ast_alloc(
        AST_WHILE,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.while_statement.condition = condition;
    node->data.while_statement.body = body;

    return node;
}

//Modificado
ASTNode *ast_create_binary(
    BinaryOperator operator,
    ASTNode *left,
    ASTNode *right,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(
        AST_BINARY_EXPRESSION,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.binary.operator = operator;
    node->data.binary.left = left;
    node->data.binary.right = right;

    return node;
}

//Modificado
ASTNode *ast_create_unary(
    UnaryOperator operator,
    ASTNode *operand,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(
        AST_UNARY_EXPRESSION,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.unary.operator = operator;
    node->data.unary.operand = operand;

    return node;
}

//Modificado
ASTNode *ast_create_identifier(
    const char *name,
    int line,
    int column
) {
    if (name == NULL) {
        return NULL;
    }

    ASTNode *node = ast_alloc(
        AST_IDENTIFIER,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.identifier.name = ast_strdup(name);

    if (node->data.identifier.name == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

//Modificado
ASTNode *ast_create_integer(
    const char *lexeme,
    int line,
    int column
) {
    if (lexeme == NULL) {
        return NULL;
    }

    ASTNode *node = ast_alloc(
        AST_INTEGER_LITERAL,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.integer_literal.lexeme = ast_strdup(lexeme);

    if (node->data.integer_literal.lexeme == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *ast_create_boolean(
    int value,
    int line,
    int column
) {
    ASTNode *node = ast_alloc(
        AST_BOOLEAN_LITERAL,
        line,
        column
    );

    if (node == NULL) {
        return NULL;
    }

    node->data.boolean_literal.value = value ? 1 : 0;

    return node;
}


//Impresión
static const char *binary_operator_to_string(BinaryOperator op) {
    switch (op) {
        case OP_ADD:           return "+";
        case OP_SUBTRACT:      return "-";
        case OP_MULTIPLY:      return "*";
        case OP_DIVIDE:        return "/";
        case OP_LESS:          return "<";
        case OP_LESS_EQUAL:    return "<=";
        case OP_GREATER:       return ">";
        case OP_GREATER_EQUAL: return ">=";
        case OP_EQUAL:         return "==";
        case OP_NOT_EQUAL:     return "!=";
        case OP_AND:           return "&&";
        case OP_OR:            return "||";
        default:               return "?";
    }
}

static const char *declared_type_to_string(ASTDeclaredType type) {
    switch (type) {
        case AST_TYPE_INT:
            return "int";

        case AST_TYPE_BOOL:
            return "bool";

        default:
            return "?";
    }
}

static void ast_print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static void ast_print_node(const ASTNode *node, int indent) {
    if (node == NULL) {
        ast_print_indent(indent);
        printf("(null)\n");
        return;
    }

    switch (node->type) {

        case AST_PROGRAM:
            ast_print_indent(indent);
            printf("PROGRAM\n");

            for (si

void ast_print(const ASTNode *node) {
    ast_print_node(node, 0);
}

void ast_destroy(ASTNode *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {

        case AST_PROGRAM:
            ast_node_list_destroy(
                &node->data.program.statements
            );
            break;


        case AST_BLOCK:
            ast_node_list_destroy(
                &node->data.block.statements
            );
            break;


        case AST_VARIABLE_DECLARATION:
            free(node->data.variable_declaration.name);

            ast_destroy(
                node->data.variable_declaration.initializer
            );
            break;
        case AST_ASSIGNMENT:
            free(node->data.assignment.name);

            ast_destroy(
                node->data.assignment.value
            );
            break;


        case AST_PRINT:
            ast_destroy(
                node->data.print_statement.expression
            );
            break;


        case AST_IF:
            ast_destroy(
                node->data.if_statement.condition
            );

            ast_destroy(
                node->data.if_statement.then_branch
            );

            ast_destroy(
                node->data.if_statement.else_branch
            );
            break;

        case AST_WHILE:
            ast_destroy(
                node->data.while_statement.condition
            );

            ast_destroy(
                node->data.while_statement.body
            );
            break;


        case AST_BINARY_EXPRESSION:
            ast_destroy(
                node->data.binary.left
            );

            ast_destroy(
                node->data.binary.right
            );
            break;


        case AST_UNARY_EXPRESSION:
            ast_destroy(
                node->data.unary.operand
            );
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
