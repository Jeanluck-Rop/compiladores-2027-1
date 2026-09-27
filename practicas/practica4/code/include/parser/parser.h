#ifndef MINIC_PARSER_H
#define MINIC_PARSER_H

#include "lexer/lexer.h"

/* Resultado del analisis completo de un programa */
typedef enum {
    PARSE_OK,              /* programa sintacticamente correcto */
    PARSE_INVALID,         /* hubo errores lexicos o sintacticos */
    PARSE_INTERNAL_FAILURE /* fallo de E/S o de memoria al pedir tokens */
} ParseResult;

typedef struct {
    Lexer *lexer;
    Token current;        /* token de anticipacion */
    Token previous;       /* ultimo token consumido */
    int has_current;
    int has_previous;
    int had_error;        /* hubo algun error lexico o sintactico */
    int panic_mode;       /* suprime errores en cascada hasta sincronizar */
    int internal_failure; /* el lexer fallo; el analisis se detiene */
} Parser;


int parser_init(Parser *parser, Lexer *lexer);
ParseResult parser_parse_program(Parser *parser);
void parser_destroy(Parser *parser);

#endif
