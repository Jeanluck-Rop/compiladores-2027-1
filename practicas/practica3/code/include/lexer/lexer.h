#ifndef MINIC_LEXER_H
#define MINIC_LEXER_H

#include "token.h"
#include <stdio.h>

/* Estructura con la que podemos llevar un mejor flujo del proceso del lexer */
typedef struct {
    FILE *file;
    size_t line;
    size_t column;
    int last_was_cr;
    int current;
} Lexer;

/* Funcion que comienza el escaneo de un archivo para ser procesado por el lexer */
int lexer_scan(FILE *file);

#endif
