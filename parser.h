#ifndef _PARSER_H
#define _PARSER_H

#include "ast.h"
#include "tokeniser.h"

typedef struct
{
    int idx;
    Token *tokens;
} Parser;

AbstractNode *parse_term(Parser *parser);
AbstractNode *parse_expression(Parser *parser);
AbstractNode *parse_factor(Parser *parser);
AbstractNode *parse(Parser *parser);

Parser new_parser(Token *tokens);
void expect(Parser *parser, TokenType token_type, string error_msg);
void throw_parser_error(Parser *parser, string error_msg);

Token peek(Parser *parser);
Token look_ahead(Parser *parser);
Token eat(Parser *parser);
#endif
