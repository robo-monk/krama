#include "Parser.h"
#include "Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"

Token look_ahead(Parser *parser) { return parser->tokens[parser->idx + 1]; }

Token peek(Parser *parser) { return parser->tokens[parser->idx]; }

Token eat(Parser *parser) { return parser->tokens[parser->idx++]; }

void throw_parser_error(Parser *parser, string error_msg) {
  printf("[1:%d] ", parser->idx);
  printf("Parser Error: %s \n", error_msg);
  exit(1);
}

void expect(Parser *parser, TokenType token_type, string error_msg) {
  if (peek(parser).type != token_type) {
    throw_parser_error(parser, error_msg);
  }
}

Parser new_parser(Token *tokens) {
  Parser p = {.idx = 0, .tokens = tokens};

  return p;
}

Statement *parse_factor(Parser *parser) {
  Token current_token = eat(parser);

  if (current_token.type == IDENTIFIER) {
    return new_var_read_stmt(LiteralType_i32, current_token.value.str_value,
                             current_token);
  } else if (current_token.type == NUMBER) {
    // return new_abstract_node(NULL, current_token, NULL);
    return new_i32_literal_stmt(current_token.value.i32_value, current_token);
  } else if (current_token.type == OPEN_PAR) {
    Statement *node = parse_expression(parser);
    expect(parser, CLOSE_PAR, "Expected closing paren");
    eat(parser);
    return node;
  }

  printf("\n ERROR:");
  dbg_token(current_token);
  printf("  |   ");
  throw_parser_error(parser, "Unexpected token: Expected Factor");
  return NULL;
}

Statement *parse_term(Parser *parser) {
  Statement *stmt = parse_factor(parser);
  Token current_token = peek(parser);

  while (current_token.type == OP && (current_token.value.op_type == MUL ||
                                      current_token.value.op_type == DIV)) {

    eat(parser);
    Statement *right = parse_factor(parser);
    stmt = new_bin_expr_stmt(current_token.value.op_type, stmt, right,
                             current_token);
    current_token = peek(parser);
  }

  return stmt;
}

Statement *parse_expression(Parser *parser) {

  // printf("\nimma call parse_term being a bitch\n");
  // dbg_token(peek(parser));
  // printf("\n");

  Statement *stmt = parse_term(parser);

  Token current_token = peek(parser);

  while (current_token.type == OP && (current_token.value.op_type == ADD ||
                                      current_token.value.op_type == MIN)) {
    eat(parser);
    Statement *right = parse_term(parser);
    stmt = new_bin_expr_stmt(current_token.value.op_type, stmt, right,
                             current_token);
    current_token = peek(parser);
  }

  return stmt;
}

Statement *parse_statement(Parser *parser) {
  Token current_token = peek(parser);

  if (current_token.type == LET) {
    eat(parser);
    expect(parser, IDENTIFIER, "expected identifier");
    Token identifier = eat(parser);
    // dbg_token(identifier);
    expect(parser, EQ, "expected assigment");
    eat(parser);

    Statement *decl =
        new_var_decl_stmt(LiteralType_i32, identifier.value.str_value,
                          parse_expression(parser), identifier);
    return decl;
  } else if (current_token.type == IDENTIFIER) {
    Token identifier = eat(parser);
    if (peek(parser).type == EQ) {
      eat(parser);
      return new_var_write_stmt(LiteralType_i32, identifier.value.str_value,
                                parse_expression(parser), identifier);
    } else {
      return new_var_read_stmt(LiteralType_i32, identifier.value.str_value,
                               identifier);
    }
  }

  if (current_token.type == PROGRAM_END)
    return NULL;

  return parse_expression(parser);
}

Statement *parse(Parser *parser) {
  Statement *stmt = parse_statement(parser);

  // if (peek(parser).type != PROGRAM_END) {
  // throw_parser_error(parser, "premature exit.");
  // }

  return stmt;
}

Program *parse_program(Token *tokens) {
  Parser parser = new_parser(tokens);
  Program *program = new_program();

  Statement *stmt = parse(&parser);

  do {
    // printf("\n-> EXEC   | \n");
    // dbg_stmt(stmt);
    // printf("\n");
    push_stmt(stmt, program);
    stmt = parse(&parser);
  } while (stmt != NULL);

  return program;
}
