#include "lexer/lexer.h"
#include "lexer/keywords.h"
#include "lexer/buffer_manage.h"

#include <ctype.h>
#include <string.h>


/* Checamos si el caracter leido es un espacio en blanco (o salto de linea)*/
static int
is_ignored_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}


/* Construye un token 'out', solo falla en memoria */
static LexerStatus
make_token(Token *out,
           TokenType type,
           const char *lexeme,
           size_t line,
           size_t column)
{
    if (!token_init(out, type, lexeme, line, column))
        return LEXER_STATUS_MEMORY_ERROR;
    return LEXER_STATUS_OK;
}


/* Devuelve el caracter aun no consumido, sin leer del archivo */
static int
lexer_check(Lexer *lex)
{
    return lex->current;
}


/* Consumimos el caracter actual, actualizando linea/columna y precargando el siguiente */
static int
lexer_advance(Lexer *lex)
{
    int c = lexer_check(lex);
    
    if (c != EOF) {
        if (c == '\r'){
            (lex->line)++;
            lex->column = 0;
            lex->last_was_cr = 1;
        } else if (c == '\n') {
            if (lex->last_was_cr) {
                lex->last_was_cr = 0;
            } else {
                (lex->line)++;
                lex->column = 0;
            }
        } else {
            (lex->column)++;
            lex->last_was_cr = 0;
        }
        lex->current = fgetc(lex->file);
    }
    
    return c;
}


/* Escaneamos caracter por caracter en busca de un identificador o palabra reservada */
static LexerStatus
scan_keyword(Lexer *lex,
             int first_char,
             size_t token_line,
             size_t token_column,
             Token *out)
{
    TextBuffer buffer;
    TokenType kw_type;
    LexerStatus status;

    if (!buffer_init(&buffer, 16))
        return LEXER_STATUS_MEMORY_ERROR;
    
    if (!buffer_append(&buffer, (char)first_char)) {
        buffer_free(&buffer);
        return LEXER_STATUS_MEMORY_ERROR;
    }

    while (lexer_check(lex) != EOF && (isalnum(lexer_check(lex)) || lexer_check(lex) == '_')) {
        if (!buffer_append(&buffer, (char)lexer_advance(lex))) {
            buffer_free(&buffer);
            return LEXER_STATUS_MEMORY_ERROR;
        }
    }

    if (!lookup_keyword(buffer.data, &kw_type))
        kw_type = IDENTIFIER;

    /* token_init copia el lexema, por eso podemos liberar el buffer */
    status = make_token(out, kw_type, buffer.data, token_line, token_column);
    buffer_free(&buffer);
    return status;
}


/* Escaneamos caracter por caracter en busca de un entero */
static LexerStatus
scan_integer(Lexer *lex,
             int first_char,
             size_t token_line,
             size_t token_column,
             Token *out)
{
    TextBuffer buffer;
    LexerStatus status;
    
    if (!buffer_init(&buffer, 16))
        return LEXER_STATUS_MEMORY_ERROR;
    
    if (!buffer_append(&buffer, (char)first_char)) {
        buffer_free(&buffer);
        return LEXER_STATUS_MEMORY_ERROR;
    }
    
    while (lexer_check(lex) != EOF && isdigit(lexer_check(lex))) {
        if (!buffer_append(&buffer, (char)lexer_advance(lex))) {
            buffer_free(&buffer);
            return LEXER_STATUS_MEMORY_ERROR;
        }
    }
    
    status = make_token(out, INTEGER, buffer.data, token_line, token_column);
    buffer_free(&buffer);
    return status;
}


/*
 * Comprobamos si =,<,>,!,&,| forman un operador compuesto (==, <=, >=, !=, &&, ||)
 * o quedan como su version simple.
 */
static TokenType
scan_operator(Lexer *lex,
              int c,
              char* lexeme)
{
    int next = lexer_check(lex);
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

    lexeme[0] = (char)c;
    if (consumed_next) {
        lexer_advance(lex);
        lexeme[1] = (char)next;
        lexeme[2] = '\0';
    } else {
        lexeme[1] = '\0';
    }

    return type;
}


/* Escaneamos un caracter para verificar que tipo de simbolo simple es */
static int
scan_simple(int c,
            TokenType *type)
{
    if (type == NULL)
        return 0;
    
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


/* Descartamos el contenido de un comentario hasta \n, \r o EOF */
static void
skip_comment_content(Lexer *lex)
{    
    while (lexer_check(lex) != EOF && lexer_check(lex) != '\n' && lexer_check(lex) != '\r')
        lexer_advance(lex);
}


/* Preparamos el lexer, precargando el primer caracter del archivo */
int
lexer_init(Lexer *lexer,
           FILE *source)
{
    if (lexer == NULL || source == NULL)
        return 0;

    lexer->file = source;
    lexer->line = 1;
    lexer->column = 0;
    lexer->last_was_cr = 0;
    lexer->current = fgetc(source);
    return 1;
}


/* Producimos exactamente el siguiente token, sin imprimirlo */
LexerStatus
lexer_next_token(Lexer *lexer,
                 Token *out)
{
    if (lexer == NULL || lexer->file == NULL || out == NULL)
        return LEXER_STATUS_IO_ERROR;
    
    
    while (1) {
        int c = lexer_check(lexer);
        size_t token_line;
        size_t token_column;
        TokenType type;
        
        if (c == EOF) {
            if (ferror(lexer->file))
                return LEXER_STATUS_IO_ERROR;
            return make_token(out, TOKEN_EOF, "", lexer->line, lexer->column);
        }

        token_line = lexer->line;
        token_column = lexer->column;
        lexer_advance(lexer);
        
        if (is_ignored_space(c))
            continue;

        //Comentarios o slash
        if (c == '/') {
            if (lexer_check(lexer) == '/') {
                lexer_advance(lexer);
                skip_comment_content(lexer);
                continue;
            }
            return make_token(out, SLASH, "/", token_line, token_column);
        }

        //Operadores
        if (c != '\0' && strchr("=<>!&|", c) != NULL) {
            char lexeme[3];
            type = scan_operator(lexer, c, lexeme);
            return make_token(out, type, lexeme, token_line, token_column);
        }

        //Numeros enteros
        if (isdigit(c))
            return scan_integer(lexer, c, token_line, token_column, out);
        
        //Identificadores y palabras reservadas
        if (isalpha(c) || c == '_')
            return scan_keyword(lexer, c, token_line, token_column, out);
        
        //Simples y errores
        char lexeme[2] = {(char)c, '\0'};
        if (!scan_simple(c, &type))
            type = ERROR;
        return make_token(out, type, lexeme, token_line, token_column);
    }
}


/* El lexer no reserva memoria propia ni es propietario del archivo */
void
lexer_destroy(Lexer *lexer)
{
    if (lexer == NULL)
        return;
    lexer->file = NULL;
    lexer->current = EOF;
}


/* Imprimimos la secuencia completa de tokens (como registro de lo echo en las practicas 1 y 2) */
int
lexer_print_tokens(FILE *source)
{
    Lexer lexer;
    Token token;
    int done = 0;
    
    if (!lexer_init(&lexer, source))
        return 2;
    
    while (!done) {
        LexerStatus status = lexer_next_token(&lexer, &token);

        if (status == LEXER_STATUS_MEMORY_ERROR) {
            fprintf(stderr, "Error: no se pudo reservar memoria.\n");
            lexer_destroy(&lexer);
            return 2;
        }
        if (status == LEXER_STATUS_IO_ERROR) {
            fprintf(stderr, "Error: no se pudo leer el archivo.\n");
            lexer_destroy(&lexer);
            return 2;
        }
        
        token_print(&token);
        done = (token.type == TOKEN_EOF);
        token_destroy(&token);
    }
    
    lexer_destroy(&lexer);
    return 0;
}
