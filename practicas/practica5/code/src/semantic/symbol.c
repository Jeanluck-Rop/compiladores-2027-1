#include <stdlib.h>
#include <string.h>

#include "semantic/symbol.h"

Symbol*
symbol_create(const char *name,
              Type type,
              int scope_level,
              int line,
              int column,
              ASTNode *declaration)
{
    Symbol *symbol;
    size_t len;
    
    if (name == NULL)
        return NULL;
    
    symbol = calloc(1, sizeof *symbol);
    if (symbol == NULL)
        return NULL;
    
    len = strlen(name) + 1;
    symbol->name = malloc(len);
    if (symbol->name == NULL) {
        free(symbol);
        return NULL;
    }
    
    memcpy(symbol->name, name, len);
    symbol->type = type;
    symbol->scope_level = scope_level;
    symbol->line = line;
    symbol->column = column;
    symbol->declaration = declaration;
    return symbol;
}

void
symbol_destroy(Symbol *symbol)
{
    if (symbol == NULL)
        return;
    free(symbol->name);
    free(symbol);
}
