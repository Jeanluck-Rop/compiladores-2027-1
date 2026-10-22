#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "token.h"

/* Representacion de una palabra reservada para un mapeo adecuado */
typedef struct {
    const char *lexeme;
    TokenType type;
} Keyword;

/* Buscamos el lexema con el mapeo anterior y si existe devolvemos su TokenType */
int lookup_keyword(const char *lexeme, TokenType *type);

#endif
