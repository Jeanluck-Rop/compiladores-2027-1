#include <stdlib.h>
#include <string.h>

#include "semantic/scope.h"

#define SCOPE_INITIAL_BUCKETS 16

static size_t
hash_name(const char *s)
{
    size_t h = 5381;
    while (*s != '\0')
        h = h * 33 + (unsigned char)*s++;
    return h;
}

Scope*
scope_create(int level,
             Scope *parent)
{
    Scope *scope = calloc(1, sizeof *scope);
    if (scope == NULL)
        return NULL;
    
    scope->buckets = calloc(SCOPE_INITIAL_BUCKETS, sizeof *scope->buckets);

    if (scope->buckets == NULL) {
        free(scope);
        return NULL;
    }
    
    scope->bucket_count = SCOPE_INITIAL_BUCKETS;
    scope->level = level;
    scope->parent = parent;
    return scope;
}

void
scope_destroy(Scope *scope)
{
    size_t i;
    if (scope == NULL)
        return;
    
    for (i = 0; scope->buckets != NULL && i < scope->bucket_count; i++) {
        Symbol *s = scope->buckets[i];
        while (s != NULL) {
            Symbol *next = s->next;
            symbol_destroy(s);
            s = next;
        }
    }
    free(scope->buckets);
    free(scope);
}

Symbol*
scope_lookup(const Scope *scope,
             const char *name)
{
    Symbol *s;
    if (scope == NULL || name == NULL)
        return NULL;
    for (s = scope->buckets[hash_name(name) % scope->bucket_count]; s != NULL; s = s->next)
        if (strcmp(s->name, name) == 0)
            return s;
    return NULL;
}

static int
scope_grow(Scope *scope)
{
    size_t new_count = scope->bucket_count * 2;
    size_t i;
    Symbol **nb = calloc(new_count, sizeof *nb);
    
    if (nb == NULL) 
        return 0;
    
    for (i = 0; i < scope->bucket_count; i++) {
        Symbol *s = scope->buckets[i];
        while (s != NULL) {
            Symbol *next = s->next;
            size_t idx = hash_name(s->name) % new_count;
            s->next = nb[idx];
            nb[idx] = s;
            s = next;
        }
    }
    
    free(scope->buckets);
    scope->buckets = nb;
    scope->bucket_count = new_count;
    return 1;
}

SymbolTableResult
scope_insert(Scope *scope,
             Symbol *symbol)
{
    size_t idx;
    if (scope == NULL || symbol == NULL || symbol->name == NULL)
        return ST_INTERNAL_ERROR;
    
    if (scope_lookup(scope, symbol->name) != NULL)
        return ST_DUPLICATE;
    
    // factor de carga maximo 0.75
    if ((scope->symbol_count + 1) * 4 > scope->bucket_count * 3 && !scope_grow(scope))
        return ST_INTERNAL_ERROR;
    
    idx = hash_name(symbol->name) % scope->bucket_count;
    symbol->next = scope->buckets[idx];
    scope->buckets[idx] = symbol;
    scope->symbol_count++;
    return ST_OK;
}
