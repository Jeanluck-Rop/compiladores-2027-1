#ifndef MINIC_SYMBOL_H
#define MINIC_SYMBOL_H

#include "ast/ast.h"

typedef enum { TYPE_INT, TYPE_BOOL, TYPE_ERROR, TYPE_UNKNOWN } Type;

typedef struct Symbol {
    char *name;               /* copia propia */
    Type type;
    int scope_level;
    int line;
    int column;
    ASTNode *declaration;     /* NO es propiedad del simbolo */
    struct Symbol *next;      /* encadenamiento en el bucket de la hash */
} Symbol;

Symbol *symbol_create(const char *name, Type type, int scope_level,
                      int line, int column, ASTNode *declaration);
void symbol_destroy(Symbol *symbol);   /* acepta NULL; no libera el AST */

#endif
