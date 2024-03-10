#ifndef _PARSER_H
#define _PARSER_H

#include "../ast/AbstractSyntaxTree.h"
#include "Tokeniser.h"

typedef struct {
  int idx;
  Token *tokens;
} Parser;

Statement *parse_term(Parser *parser);
Statement *parse_expression(Parser *parser);
Statement *parse_factor(Parser *parser);
Statement *parse(Parser *parser);

Parser new_parser(Token *tokens);
void expect(Parser *parser, TokenType token_type, string error_msg);
void throw_parser_error(Parser *parser, string error_msg);

Token peek(Parser *parser);
Token look_ahead(Parser *parser);
Token eat(Parser *parser);

BlockStatement *parse_program(Token *tokens);
#endif
