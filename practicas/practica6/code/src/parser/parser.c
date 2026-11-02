#include "parser/parser.h"
#include "parser/grammar.h"

#include <stdio.h>
#include <string.h>

/* Reportamos en stderr un fallo interno del lexer (memoria o lectura) */
static void
report_lexer_failure(LexerStatus status)
{
    if (status == LEXER_STATUS_MEMORY_ERROR)
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
    else
        fprintf(stderr, "Error: no se pudo leer el archivo.\n");
}

/* Pedimos tokens al lexer, reportando y descartando los ERROR, hasta obtener uno valido */
static int
fetch_token(Parser *parser)
{
    while (1) {
        Token token;
        LexerStatus status = lexer_next_token(parser->lexer, &token);

        if (status != LEXER_STATUS_OK) {
            report_lexer_failure(status);
            parser->internal_failure = 1;
            return 0;
        }

        if (token.type != ERROR) {
            parser->current = token;
            parser->has_current = 1;
            return 1;
        }

        fprintf(stderr, "Error lexico [%zu:%zu]: caracter invalido '%s'.\n",
                token.line, token.column, token.lexeme);
        parser->had_error = 1;
        token_destroy(&token);
    }
}

/* Consumimos el token actual, moviendolo a previous, y precargamos el siguiente */
int
parser_advance(Parser *parser)
{
    if (parser == NULL
        || parser->lexer == NULL
        || parser->internal_failure)
        return 0;

    if (parser->has_current && parser->current.type == TOKEN_EOF)
        return 1;

    if (parser->has_previous) {
        token_destroy(&parser->previous);
        parser->has_previous = 0;
    }

    if (parser->has_current) {
        parser->previous = parser->current;
        parser->has_previous = 1;
        parser->has_current = 0;
    }

    return fetch_token(parser);
}

/* Devolvemos el ultimo token consumido, o NULL si aun no hay ninguno */
const Token*
parser_previous(const Parser *parser)
{
    if (parser == NULL || !parser->has_previous)
        return NULL;
    return &parser->previous;
}

/* Checamos si el token actual es del tipo indicado, sin consumirlo */
int
parser_check(const Parser *parser,
             TokenType type)
{
    return parser != NULL
        && parser->has_current
        && parser->current.type == type;
}

/* Consumimos el token actual solo si es del tipo indicado */
int
parser_match(Parser *parser,
             TokenType type)
{
    if (!parser_check(parser, type))
        return 0;
    parser_advance(parser);
    return 1;
}

/* Checamos si ya no queda nada por analizar (TOKEN_EOF o fallo interno) */
int
parser_end(const Parser *parser)
{
    return parser == NULL
        || parser->internal_failure
        || !parser->has_current
        || parser->current.type == TOKEN_EOF;
}

/* Reportamos un error sintactico sobre un token y activamos panic_mode para evitar cascadas */
void
parser_error(Parser *parser,
             const Token *token,
             const char *what)
{
    if (parser == NULL
        || token == NULL
        || parser->internal_failure)
        return;

    parser->had_error = 1;

    if (parser->panic_mode)
        return;
    parser->panic_mode = 1;

    if (token->type == TOKEN_EOF)
        fprintf(stderr,
                "Error sintactico [%zu:%zu]: se esperaba %s, pero se encontro TOKEN_EOF.\n",
                token->line, token->column, what);
    else
        fprintf(stderr,
                "Error sintactico [%zu:%zu]: se esperaba %s, pero se encontro '%s'.\n",
                token->line, token->column, what, token->lexeme);
}

/* Reportamos un error sintactico sobre el token actual */
void
parser_current_error(Parser *parser,
                     const char *what)
{
    if (parser == NULL || !parser->has_current)
        return;
    parser_error(parser, &parser->current, what);
}

/* Exigimos un token del tipo esperado; si no esta, reportamos sin fingir que existia */
int
parser_consume(Parser *parser,
               TokenType expected,
               const char *what)
{
    if (parser_check(parser, expected)) {
        parser_advance(parser);
        return 1;
    }

    parser_current_error(parser, what);
    return 0;
}

/* Descartamos tokens hasta un punto seguro para continuar el analisis tras un error */
void
parser_sync(Parser *parser)
{
    if (parser == NULL)
        return;

    parser->panic_mode = 0;

    while (!parser->internal_failure
           && parser->has_current) {
        switch (parser->current.type) {
        case SEMICOLON:
            parser_advance(parser);
            return;

        case INT:
        case BOOL:
        case IF:
        case WHILE:
        case PRINT:
        case IDENTIFIER:
        case LBRACE:
            return;

        case RBRACE:
        case ELSE:
        case TOKEN_EOF:
            return;

        default:
            parser_advance(parser);
            break;
        }
    }
}

/* Preparamos el parser, precargando el primer token de anticipacion */
int
parser_init(Parser *parser,
            Lexer *lexer)
{
    if (parser == NULL)
        return 0;

    memset(parser, 0, sizeof(*parser));

    if (lexer == NULL)
        return 0;

    parser->lexer = lexer;
    return fetch_token(parser);
}

/* Analizamos el programa completo y traducimos el estado final a un ParseResult */
ParseResult
parser_parse_program(Parser *parser, ASTNode **out_root)
{
    ASTNode *root;

    if (out_root == NULL)
        return PARSE_INTERNAL_FAILURE;
    *out_root = NULL;

    if (parser == NULL || parser->lexer == NULL || !parser->has_current)
        return PARSE_INTERNAL_FAILURE;

    root = parse_program(parser);

    if (parser->internal_failure) {
        ast_destroy(root);
        return PARSE_INTERNAL_FAILURE;
    }
    if (parser->had_error || root == NULL) {
        ast_destroy(root);
        return parser->had_error ? PARSE_INVALID : PARSE_INTERNAL_FAILURE;
    }

    *out_root = root;
    return PARSE_OK;
}

/* Liberamos los tokens que conserva el parser; el lexer no le pertenece */
void
parser_destroy(Parser *parser)
{
    if (parser == NULL)
        return;

    if (parser->has_current) {
        token_destroy(&parser->current);
        parser->has_current = 0;
    }
    if (parser->has_previous) {
        token_destroy(&parser->previous);
        parser->has_previous = 0;
    }
    parser->lexer = NULL;
}
