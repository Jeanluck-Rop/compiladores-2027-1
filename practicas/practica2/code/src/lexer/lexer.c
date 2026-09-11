#include "lexer/lexer.h"
#include "lexer/keywords.h"

#include <ctype.h>
#include <string.h>

/* Checamos si el caracter leido es un espacio en blanco (o salto de linea)*/
static int
is_ignored_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* Funcion auxiliar que construye, imprime y libera un token */
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

/* Inicializamos la estructura lexer para empezar a leer el archivo */
static void
lexer_init(Lexer *lex,
           FILE *file)
{
    lex->file = file;
    lex->line = 1;
    lex->column = 0;
    lex->last_was_cr = 0;
    //
    lex->current = fgetc(file);
}

/* Devuelve el caracter aun no consumido, sin leer del archivo */
static int
lexer_peek(Lexer *lex)
{
    return lex->current;
}

/* Leemos un caracter del archivo y actualiza linea/columna segun corresponda */
static int
lexer_advance(Lexer *lex)
{
    int c = lex->current;
    
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
static int
scan_keyword(Lexer *lex,
             int first_char,
             size_t token_line,
             size_t token_column)
{
    char buffer[64];
    size_t len = 0;
    TokenType kw_type;
    
    buffer[len++] = (char)first_char;
    while (lexer_peek(lex) != EOF &&
           (isalnum(lexer_peek(lex)) || lexer_peek(lex) == '_')) {
        if (len + 1 >= sizeof(buffer)) {
            fprintf(stderr, "Error: identificador demasiado largo.\n");
            return 0;
        }
        buffer[len++] = (char)lexer_advance(lex);
    }
    buffer[len] = '\0';

    if (lookup_keyword(buffer, &kw_type))
        return emit_token(kw_type, buffer, token_line, token_column);
    
    return emit_token(IDENTIFIER, buffer, token_line, token_column);
}

/* Escaneamos caracter por caracter en busca de un entero */
static int
scan_integer(Lexer *lex,
             int first_char,
             size_t token_line,
             size_t token_column)
{
    char buffer[64];
    size_t len = 0;

    buffer[len++] = (char)first_char;
    while (lexer_peek(lex) != EOF && isdigit(lexer_peek(lex))) {
        if (len + 1 >= sizeof(buffer)) {
            fprintf(stderr, "Error: número demasiado largo.\n");
            return 0;
        }
        buffer[len++] = (char)lexer_advance(lex);
    }
    buffer[len] = '\0';
        
    return emit_token(INTEGER, buffer, token_line, token_column);
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
    int next = lexer_peek(lex);
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
        lexer_advance(lex);
        lexeme[0] = (char)c;
        lexeme[1] = (char)next;
        lexeme[2] = '\0';
    } else {
        lexeme[0] = (char)c;
        lexeme[1] = '\0';;
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

/* Descartamos el contenido de un comentario hasta \n o EOF */
static void
skip_comment_content(Lexer *lex)
{
    while (lexer_peek(lex) != EOF && lexer_peek(lex) != '\n')
        lexer_advance(lex);
    if (lexer_peek(lex) == '\n')
        lexer_advance(lex);
}

/* Funcion principal que comienza el escaneo de un archivo para ser procesado por el lexer */
int
lexer_scan(FILE *file)
{
    int c;
    Lexer lex;

    if (file == NULL)
        return 2;

    lexer_init(&lex, file);
    
    while ((c = lexer_peek(&lex)) != EOF) {
        size_t token_line = lex.line;
        size_t token_column = lex.column;
        TokenType type;

        lexer_advance(&lex);
                
        if (is_ignored_space(c))
            continue;

        //Comentarios
        if (c == '/') {
            if (lexer_peek(&lex) == '/') {
                lexer_advance(&lex);
                skip_comment_content(&lex);
                continue;
            }
            if (!emit_token(SLASH, "/", token_line, token_column))
                return 2;
            continue;
        }

        //Operadores
        if (strchr("=<>!&|", c) != NULL) {
            char lexeme[3];
            type = scan_operator(&lex, c, lexeme);
            if (!emit_token(type, lexeme, token_line, token_column))
                return 2;
            continue;
        }

        //Numeros enteros
        if (isdigit(c)) {
            if (!scan_integer(&lex, c, token_line, token_column))
                return 2;
            continue;
        }
        
        //Identificadores y palabras reservadas
        if (isalpha(c) || c == '_') {
            if (!scan_keyword(&lex, c, token_line, token_column))
                return 2;
            continue;
        }
        
        //Simples
        if (scan_simple(c, &type)) {
            char lexeme[2] = {(char)c, '\0'};
            if (!emit_token(type, lexeme, token_line, token_column))
                return 2;
            continue;
        }        

        //Errores lexicos
        char lexeme[2] = {(char)c, '\0'};
         if (!emit_token(ERROR, lexeme, token_line, token_column))
             return 2;
    }

    if (ferror(file)) {
        fprintf(stderr, "Error: no se pudo leer el archivo.\n");
        return 2;
    }

    if (!emit_token(TOKEN_EOF, "", lex.line, lex.column))
        return 2;
    
    return 0;
}
