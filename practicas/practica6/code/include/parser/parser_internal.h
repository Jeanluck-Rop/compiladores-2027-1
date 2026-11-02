#ifndef MINIC_PARSER_INTERNAL_H
#define MINIC_PARSER_INTERNAL_H

#include "parser/parser.h"

int parser_advance(Parser *parser);
int parser_check(const Parser *parser, TokenType type);
int parser_match(Parser *parser, TokenType type);
int parser_consume(Parser *parser, TokenType expected, const char *what);
int parser_end(const Parser *parser);
const Token* parser_previous(const Parser *parser);
void parser_error(Parser *parser, const Token *token, const char *what);
void parser_current_error(Parser *parser, const char *what);
void parser_sync(Parser *parser);

#endif
