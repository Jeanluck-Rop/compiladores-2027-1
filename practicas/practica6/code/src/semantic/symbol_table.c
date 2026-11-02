#include "semantic/symbol_table.h"

SymbolTableResult
symbol_table_init(SymbolTable *table)
{
    if (table == NULL)
        return ST_INTERNAL_ERROR;
    
    table->current_scope = NULL;
    table->scope_count = 0;
    table->current_scope = scope_create(0, NULL);

    if (table->current_scope == NULL)
        return ST_INTERNAL_ERROR;
    
    table->scope_count = 1;
    return ST_OK;
}

SymbolTableResult
symbol_table_enter_scope(SymbolTable *table)
{
    Scope *child;
    if (table == NULL || table->current_scope == NULL)
        return ST_INTERNAL_ERROR;
    
    child = scope_create(table->current_scope->level + 1, table->current_scope);

    if (child == NULL)
        return ST_INTERNAL_ERROR; //current_scope no cambia
    
    table->current_scope = child;
    table->scope_count++;
    return ST_OK;
}

SymbolTableResult
symbol_table_exit_scope(SymbolTable *table)
{
    Scope *old;
    if (table == NULL || table->current_scope == NULL)
        return ST_INTERNAL_ERROR;
    
    if (table->current_scope->parent == NULL)
        return ST_AT_GLOBAL;
    
    old = table->current_scope;
    table->current_scope = old->parent;
    table->scope_count--;
    scope_destroy(old);
    return ST_OK;
}

Symbol*
symbol_table_lookup_current(SymbolTable *table,
                            const char *name)
{
    if (table == NULL)
        return NULL;
    return scope_lookup(table->current_scope, name);
}

Symbol*
symbol_table_lookup(SymbolTable *table,
                    const char *name)
{
    Scope *s;
    if (table == NULL)
        return NULL;
    
    for (s = table->current_scope; s != NULL; s = s->parent) {
        Symbol *found = scope_lookup(s, name);
        if (found != NULL)
            return found;
    }
    return NULL;
}

SymbolTableResult
symbol_table_insert(SymbolTable *table,
                    Symbol *symbol)
{
    if (table == NULL || table->current_scope == NULL)
        return ST_INTERNAL_ERROR;
    return scope_insert(table->current_scope, symbol);
}

void
symbol_table_destroy(SymbolTable *table)
{
    if (table == NULL)
        return;
    
    while (table->current_scope != NULL) {
        Scope *parent = table->current_scope->parent;
        scope_destroy(table->current_scope);
        table->current_scope = parent;
    }
    table->scope_count = 0;
}
