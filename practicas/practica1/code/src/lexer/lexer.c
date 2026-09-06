#include "lexer/lexer.h"

#include <ctype.h>
#include <stdio.h>

/* */
int
simple_token_type(int c,
                  TokenType *type)
{
    if (type == NULL) {
        return 0;
    }
    
    switch (c) {
    case '+':
        *type = PLUS;
        break;
        
        /* TODO: agregar los demás símbolos simples de la práctica. */
        
    case '-':
        *type = MINUS;
        break;
    case '*':
        *type = STAR;
        break;
    case '/':
        *type = SLASH;
        break;
    case '(':
        *type = LPAREN;
        break;
    case ')':
        *type = RPAREN;
        break;
    case '{':
        *type = LBRACE;
        break;
    case '}':
        *type = RBRACE;
        break;
    case ';':
        *type = SEMICOLON;
        break;
    default:
        return 0;
    }
    
    return 1;
}

/* */
static int
is_ignored_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* */
static void
advance_position(int c,
                 size_t *line,
                 size_t *column,
                 int *last_was_cr)
{
    /* TODO: adaptar esta lógica para \r aislado y para la secuencia \r\n. */
    if (c == 'r'){
        (*line)++;
        *column = 0;
        *last_was_cr = 1;
    } else if (c == 'n') {
        if (*last_was_cr) {
            *last_was_cr = 0;
        } else {
            (*line)++;
            *column = 0;
        }
    } else {
        (*column)++;
        *last_was_cr = 0;
    }
}

/* */
int
lexer_scan(FILE *file)
{
    size_t line = 1;
    size_t column = 0;
    int last_was_cr = 0;
    int c;

    if (file == NULL) {
        return 2;
    }

    while ((c = fgetc(file)) != EOF) {
        size_t token_line = line;
        size_t token_column = column;
        TokenType type;

        advance_position(c, &line, &column, &last_was_cr);

        if (is_ignored_space(c)) {
            continue;
        }

        if (simple_token_type(c, &type)) {
            char lexeme[2] = {(char)c, '\0'};
            Token token;

            if (!token_init(&token, type, lexeme,
                            token_line, token_column)) {
                fprintf(stderr, "Error: no se pudo reservar memoria.\n");
                return 2;
            }

            token_print(&token);
            token_destroy(&token);
            continue;
        }

        /* TODO: reconocer enteros */
        if (isdigit(c)) {
            char buffer[64];
            size_t len =  0;
            buffer[len++] = (char)c;

            int next;
            while ((next = fgetc(file)) != EOF && isdigit(next)) {
                if (len + 1 >= sizeof(buffer)) {
                    fprintf(stderr, "Error: número demasiado largo.\n ");
                    return 2;
                }
                buffer[len++] = (char)next;
                advance_position(next, &line, &column, &last_was_cr);
            }
            buffer[len] = '\0';
            
            if (next != EOF) {
                ungetc(next, file);
            }

            Token token;
            if (!token_init(&token, INTEGER, buffer, token_line, token_column)) {
                fprintf(stderr, "Error: no se pudo reservar memoria.\n");
                        return 2;                        
            }
            token_print(&token);
            token_destroy(&token);
            continue;
        }
        
        /* TODO: reemplazar este diagnóstico por un token ERROR */
        char lexeme[2] = {(char)c, '\0'};
        Token token;
        
        if (!token_init(&token, ERROR, lexeme, token_line, token_column)) {
            fprintf(stderr, "Error: no se pudo reservar memoria.\n");
            return 2;
        }
        token_print(&token);
        token_destroy(&token);
        continue;
    }

    if (ferror(file)) {
        fprintf(stderr, "Error: no se pudo leer el archivo.\n");
        return 2;
    }

    /* TODO: generar TOKEN_EOF con la posición actual. */
    Token eof_token;
    if (!token_init(&eof_token, TOKEN_EOF, "", line, column)) {
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
        return 2;
    }
    token_print(&eof_token);
    token_destroy(&eof_token);
    return 0;
}
