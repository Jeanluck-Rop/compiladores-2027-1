#ifndef MINIC_SYMBOL_H
#define MINIC_SYMBOL_H

#include "ast/ast.h"

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_ERROR,
    TYPE_UNKNOWN
} Type;

typedef struct Symbol {
    char *name;
    Type type;
    int scope_level;
    int line;
    int column;
    ASTNode *declaration;
    struct Symbol *next;
} Symbol;

Symbol *symbol_create(const char *name, Type type, int scope_level, int line, int column, ASTNode *declaration);
void symbol_destroy(Symbol *symbol);

#endif
