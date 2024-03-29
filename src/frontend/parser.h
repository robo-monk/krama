#ifndef _PARSER_H
#define _PARSER_H

#include "../ast/AbstractSyntaxTree.h"
#include "tokeniser.h"

typedef struct {
  int idx;
  Token *tokens;
} Parser;

Statement *parse_block(Parser *parser);
Statement *parse_term(Parser *parser);
Statement *parse_expression(Parser *parser);
Statement *parse_factor(Parser *parser);
Statement *parse(Parser *parser);

Parser new_parser(Token *tokens);
void expect(Parser *parser, TokenType token_type, string error_msg);
void throw_parser_error(Parser *parser, string error_msg);
// void report_syntax_error(Token token, string error_msg);

Token peek(Parser *parser);
Token look_ahead(Parser *parser);
Token eat(Parser *parser);

BlockStatement *parse_program(Token *tokens);
BlockStatement *program_from_tokeniser(Tokeniser *t);
BlockStatement *program_from_filename(const string filename);
void report_syntax_error(Token token, string error_msg);
string line_of_token(Token token);
#endif
