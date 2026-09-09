#include "lexer/lexer.h"

#include <ctype.h>
#include <string.h>

/* */
typedef struct {
    const char *lexeme;
    TokenType type;
} Keyword;

/* */
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

/* */
static int
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
    if (c == '\r'){
        (*line)++;
        *column = 0;
        *last_was_cr = 1;
    } else if (c == '\n') {
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
static int
emit_token(TokenType type,
           const char *lexeme,
           size_t line,
           size_t column)
{
    Token token;
    
    if (!token_init(&token, type, lexeme, line, column)) {
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
        return 0;
    }
    
    token_print(&token);
    token_destroy(&token);
    return 1;
}

/* */
static TokenType
composed_token_type(int c,
                    FILE *file,
                    char *lexeme,
                    size_t *line,
                    size_t *column,
                    int *last_was_cr)
{
    int next = fgetc(file);
    TokenType type;
    int consumed_next = 0;

    switch (c) {
    case '=':
        if (next == '=') {
            type = EQUAL;
            consumed_next = 1;
        }
        else
            type = ASSIGN;
        break;
    case '!':
        if (next == '=') {
            type = NOT_EQUAL;
            consumed_next = 1;
        }
        else
            type = ERROR;
        break;
    case '<':
        if (next == '=') {
            type = LESS_EQUAL;
            consumed_next = 1;
        }
        else
            type = LESS;
        break;
    case '>':
        if (next == '=') {
            type = GREATER_EQUAL;
            consumed_next = 1;
        }
        else
            type = GREATER;
        break;
    case '&':
        if (next == '&') {
            type = AND;
            consumed_next = 1;
        }
        else
            type = ERROR;
        break;
    case '|':
        if (next == '|') {
            type = OR;
            consumed_next = 1;
        }
        else
            type = ERROR;
        break;
    default:
        type = ERROR;
        break;
    }

    if (consumed_next) {
        lexeme[0] = (char)c;
        lexeme[1] = (char)next;
        lexeme[2] = '\0';
        advance_position(next, line, column, last_was_cr);
    } else {
        lexeme[0] = (char)c;
        lexeme[1] = '\0';
        if (next != EOF) {
            ungetc(next, file);
        }
    }

    return type;
}

/* */
static int
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
    case '-':
        *type = MINUS;
        break;
    case '*':
        *type = STAR;
        break;
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
int
lexer_scan(FILE *file)
{
    int c = 0;
    size_t line = 1;
    size_t column = 0;
    int last_was_cr = 0;

    if (file == NULL)
        return 2;

    while ((c = fgetc(file)) != EOF) {
        size_t token_line = line;
        size_t token_column = column;
        TokenType type;

        advance_position(c, &line, &column, &last_was_cr);

        if (is_ignored_space(c))
            continue;

        //Comentarios
        if (c == '/') {
            int next = fgetc(file);

            if (next == '/') {
                int i;
                
                advance_position(next, &line, &column, &last_was_cr);
                while ((i = fgetc(file)) != EOF && i != '\n')
                    advance_position(i, &line, &column, &last_was_cr);
                if (i == '\n')
                    advance_position(i, &line, &column, &last_was_cr);
                continue;
            }
            
            if (next != EOF)
                ungetc(next, file);
            if (!emit_token(SLASH, "/", token_line, token_column))
                return 2;
            continue;
        }

        //Compuestos
        if (strchr("=<>!&|", c) != NULL) {
            char lexeme[3];
            type = composed_token_type(c, file, lexeme, &line, &column, &last_was_cr);
            
            if (!emit_token(type, lexeme, token_line, token_column))
                return 2;
            
            continue;
        }

        //Numeros enteros
        if (isdigit(c)) {
            char buffer[64];
            size_t len = 0;
            int next;

            buffer[len++] = (char)c;
            while ((next = fgetc(file)) != EOF && isdigit(next)) {
                if (len + 1 >= sizeof(buffer)) {
                    fprintf(stderr, "Error: número demasiado largo.\n");
                    return 2;
                }
                buffer[len++] = (char)next;
                advance_position(next, &line, &column, &last_was_cr);
            }
            buffer[len] = '\0';

            if (next != EOF)
                ungetc(next, file);
            if (!emit_token(INTEGER, buffer, token_line, token_column))
                return 2;
            
            continue;
        }

        //Identificadores y palabras reservadas
        if (isalpha(c) || c == '_') {
            char buffer[64];
            size_t len = 0;
            int next;
            TokenType kw_type;

            buffer[len++] = (char)c;
            while ((next = fgetc(file)) != EOF &&
                   (isalnum(next) || next == '_')) {
                if (len + 1 >= sizeof(buffer)) {
                    fprintf(stderr, "Error: identificador demasiado largo.\n");
                    return 2;
                }
                buffer[len++] = (char)next;
                advance_position(next, &line, &column, &last_was_cr);
            }
            buffer[len] = '\0';

            if (next != EOF)
                ungetc(next, file);

            if (lookup_keyword(buffer, &kw_type)) {
                if (!emit_token(kw_type, buffer, token_line, token_column))
                    return 2;
            } else {
                if (!emit_token(IDENTIFIER, buffer, token_line, token_column))
                    return 2;
            }
            continue;
        }
        
        //Simples
        if (simple_token_type(c, &type)) {
            char lexeme[2] = {(char)c, '\0'};
            if (!emit_token(type, lexeme, token_line, token_column))
                return 2;
            continue;
        }        
        
        char lexeme[2] = {(char)c, '\0'};
         if (!emit_token(ERROR, lexeme, token_line, token_column))
             return 2;
        continue;
    }

    if (ferror(file)) {
        fprintf(stderr, "Error: no se pudo leer el archivo.\n");
        return 2;
    }

    if (!emit_token(TOKEN_EOF, "", line, column))
        return 2;
    return 0;
}
