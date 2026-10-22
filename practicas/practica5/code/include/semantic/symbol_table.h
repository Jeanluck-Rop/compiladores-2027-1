#ifndef MINIC_SYMBOL_TABLE_H
#define MINIC_SYMBOL_TABLE_H

#include "semantic/scope.h"

typedef struct {
    Scope *current_scope;
    size_t scope_count;
} SymbolTable;

SymbolTableResult symbol_table_init(SymbolTable *table);
SymbolTableResult symbol_table_enter_scope(SymbolTable *table);
SymbolTableResult symbol_table_exit_scope(SymbolTable *table);
Symbol *symbol_table_lookup_current(SymbolTable *table, const char *name);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);
SymbolTableResult symbol_table_insert(SymbolTable *table, Symbol *symbol);
void symbol_table_destroy(SymbolTable *table);   /* seguro tras init incompleto */

#endif
