#ifndef MINIC_SCOPE_H
#define MINIC_SCOPE_H

#include <stddef.h>
#include "semantic/symbol.h"

/*
 * Resultado de las operaciones de ambitos y de la tabla:
 *    ST_OK             operacion correcta
 *    ST_DUPLICATE      el nombre ya existe en el ambito actual (es recuperable)
 *    ST_INTERNAL_ERROR fallo de memoria o estado invalido (es fatal)
 *    ST_AT_GLOBAL      se intento salir del ambito global
 */
typedef enum {
    ST_OK,
    ST_DUPLICATE,
    ST_INTERNAL_ERROR,
    ST_AT_GLOBAL
} SymbolTableResult;

typedef struct Scope {
    Symbol **buckets;
    size_t bucket_count;
    size_t symbol_count;
    int level;
    struct Scope *parent;
} Scope;

Scope *scope_create(int level, Scope *parent);
void scope_destroy(Scope *scope);
Symbol *scope_lookup(const Scope *scope, const char *name);
SymbolTableResult scope_insert(Scope *scope, Symbol *symbol);

#endif
