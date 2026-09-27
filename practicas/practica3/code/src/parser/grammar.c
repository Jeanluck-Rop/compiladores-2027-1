#include "parser/grammar.h"
#include "parser/parser_internal.h"

/* Cada funcion corresponde a un no terminal de la gramatica de MiniC */
/* ---------- Sentencias ---------- */
static void parse_statement(Parser *parser);
static void parse_statement_list(Parser *parser);
static void parse_declaration(Parser *parser);
static void parse_assignment(Parser *parser);
static void parse_print_statement(Parser *parser);
static void parse_if_statement(Parser *parser);
static void parse_while_statement(Parser *parser);
static void parse_block(Parser *parser);

/* ---------- Expresiones ---------- */
static void parse_expression(Parser *parser);
static void parse_or_expression(Parser *parser);
static void parse_and_expression(Parser *parser);
static void parse_equality_expression(Parser *parser);
static void parse_comparison_expression(Parser *parser);
static void parse_additive_expression(Parser *parser);
static void parse_multiplicative_expression(Parser *parser);
static void parse_unary_expression(Parser *parser);
static void parse_primary(Parser *parser);


/* Reconocemos cero o mas sentencias hasta llegar al fin del archivo */
static void
parse_statement_list(Parser *parser)
{
    while (!parser_end(parser))
        parse_statement(parser);
}

/* Elegimos la produccion de la sentencia segun el token de anticipacion y sincronizamos si hubo error */
static void
parse_statement(Parser *parser)
{
    if (parser->internal_failure)
        return;

    switch (parser->current.type) {
    case INT:
    case BOOL:
        parse_declaration(parser);
        break;
    case IDENTIFIER:
        parse_assignment(parser);
        break;
    case PRINT:
        parse_print_statement(parser);
        break;
    case IF:
        parse_if_statement(parser);
        break;
    case WHILE:
        parse_while_statement(parser);
        break;
    case LBRACE:
        parse_block(parser);
        break;
    default:
        parser_current_error(parser, "una sentencia");
        parser_advance(parser);
        break;
    }

    if (parser->panic_mode)
        parser_sync(parser);
}

/* Reconocemos una declaracion: tipo, identificador, inicializacion opcional y ';' */
static void
parse_declaration(Parser *parser)
{
    parser_advance(parser);

    if (!parser_consume(parser, IDENTIFIER, "un identificador"))
        return;

    if (parser_match(parser, ASSIGN)) {
        parse_expression(parser);
        if (parser->panic_mode)
            return;
    }

    parser_consume(parser, SEMICOLON, "';'");
}

/* Reconocemos una asignacion: identificador, '=', expresion y ';' */
static void
parse_assignment(Parser *parser)
{
    parser_advance(parser);

    if (!parser_consume(parser, ASSIGN, "'='"))
        return;
    
    parse_expression(parser);
    if (parser->panic_mode)
        return;

    parser_consume(parser, SEMICOLON, "';'");
}

/* Reconocemos una impresion: print, expresion entre parentesis y ';' */
static void
parse_print_statement(Parser *parser)
{
    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'('"))
        return;

    parse_expression(parser);
    if (parser->panic_mode)
        return;

    if (!parser_consume(parser, RPAREN, "')'"))
        return;

    parser_consume(parser, SEMICOLON, "';'");
}

/* Reconocemos un if con else opcional, que se asocia con el if mas cercano */
static void
parse_if_statement(Parser *parser)
{
    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'(' despues de 'if'"))
        return;

    parse_expression(parser);
    if (parser->panic_mode)
        return;

    if (!parser_consume(parser, RPAREN, "')'"))
        return;

    parse_statement(parser);

    if (parser_match(parser, ELSE))
        parse_statement(parser);
}

/* Reconocemos un ciclo while: condicion entre parentesis y una sentencia como cuerpo */
static void
parse_while_statement(Parser *parser)
{
    parser_advance(parser);

    if (!parser_consume(parser, LPAREN, "'(' despues de 'while'"))
        return;

    parse_expression(parser);
    if (parser->panic_mode)
        return;

    if (!parser_consume(parser, RPAREN, "')'"))
        return;

    parse_statement(parser);
}

/* Reconocemos un bloque: sentencias entre llaves hasta encontrar '}' */
static void
parse_block(Parser *parser)
{
    parser_advance(parser);

    while (!parser_check(parser, RBRACE)
           && !parser_end(parser))
        parse_statement(parser);

    parser_consume(parser, RBRACE, "'}'");
}

/* Punto de entrada de las expresiones, empezando por el nivel de menor precedencia */
static void
parse_expression(Parser *parser)
{
    parse_or_expression(parser);
}

/* Reconocemos disyunciones '||', asociativas por la izquierda */
static void
parse_or_expression(Parser *parser)
{
    parse_and_expression(parser);
    while (!parser->panic_mode
           && parser_match(parser, OR))
        parse_and_expression(parser);
}

/* Reconocemos conjunciones '&&', asociativas por la izquierda */
static void
parse_and_expression(Parser *parser)
{
    parse_equality_expression(parser);
    while (!parser->panic_mode
           && parser_match(parser, AND))
        parse_equality_expression(parser);
}

/* Reconocemos comparaciones de igualdad '==' y '!=' */
static void
parse_equality_expression(Parser *parser)
{
    parse_comparison_expression(parser);
    while (!parser->panic_mode
           && (parser_match(parser, EQUAL)
               || parser_match(parser, NOT_EQUAL)))
        parse_comparison_expression(parser);
}

/* Reconocemos operadores relacionales '<', '<=', '>' y '>=' */
static void
parse_comparison_expression(Parser *parser)
{
    parse_additive_expression(parser);
    while (!parser->panic_mode
           && (parser_match(parser, LESS)
               || parser_match(parser, LESS_EQUAL)
               || parser_match(parser, GREATER)
               || parser_match(parser, GREATER_EQUAL)))
        parse_additive_expression(parser);
}

/* Reconocemos sumas y restas '+' y '-' */
static void
parse_additive_expression(Parser *parser)
{
    parse_multiplicative_expression(parser);
    while (!parser->panic_mode
           && (parser_match(parser, PLUS)
               || parser_match(parser, MINUS)))
        parse_multiplicative_expression(parser);
}

/* Reconocemos multiplicaciones y divisiones '*' y '/' */
static void
parse_multiplicative_expression(Parser *parser)
{
    parse_unary_expression(parser);
    while (!parser->panic_mode
           && (parser_match(parser, STAR)
               || parser_match(parser, SLASH)))
        parse_unary_expression(parser);
}

/* Reconocemos la negacion unaria '-', asociativa por la derecha mediante recursion */
static void
parse_unary_expression(Parser *parser)
{
    if (parser_match(parser, MINUS))
        parse_unary_expression(parser);
    else
        parse_primary(parser);
}

/* Reconocemos literales, identificadores o una expresion entre parentesis */
static void
parse_primary(Parser *parser)
{
    if (parser->internal_failure)
        return;

    switch (parser->current.type) {
    case INTEGER:
    case TRUE:
    case FALSE:
    case IDENTIFIER:
        parser_advance(parser);
        return;

    case LPAREN:
        parser_advance(parser);
        parse_expression(parser);
        if (parser->panic_mode)
            return;
        parser_consume(parser, RPAREN, "')'");
        return;

    default:
        parser_current_error(parser, "una expresion");
        return;
    }
}

/* Reconocemos el programa completo y verificamos que termine en TOKEN_EOF */
void
parse_program(Parser *parser)
{
    parse_statement_list(parser);
    if (!parser->internal_failure && !parser_check(parser, TOKEN_EOF))
        parser_current_error(parser, "el fin del archivo");
}
