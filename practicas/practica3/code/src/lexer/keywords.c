#include "lexer/keywords.h"

#include <string.h>

/* Mapeo de las palabras reservadas */
static const Keyword keywords[] = {
    {"int", INT},
    {"bool", BOOL},
    {"if", IF},
    {"else", ELSE},
    {"while", WHILE},
    {"print", PRINT},
    {"true", TRUE},
    {"false", FALSE}
};

/* Buscamos el lexema con el mapeo anterior y si existe devolvemos su TokenType */
int
lookup_keyword(const char *lexeme,
               TokenType *type)
{
    size_t n = sizeof(keywords) / sizeof(keywords[0]);

    for (size_t i = 0; i < n; i++) {
        if (strcmp(lexeme, keywords[i].lexeme) == 0) {
            *type = keywords[i].type;
            return 1;
        }
    }
    
    return 0;
}
