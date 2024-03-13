#include "Parser.h"
#include "Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"

Statement *parse_statement(Parser *parser);

Token look_ahead(Parser *parser) { return parser->tokens[parser->idx + 1]; }

Token peek(Parser *parser) { return parser->tokens[parser->idx]; }

Token eat(Parser *parser) { return parser->tokens[parser->idx++]; }

void throw_parser_error(Parser *parser, string error_msg) {
  printf("[1:%d] ", parser->idx);
  printf("Parser Error: %s \n", error_msg);
  exit(1);
}

void throw_unexpected_token(Parser *parser, string error_msg) {
  // printf("[_:%d] ", parser->idx);
  printf("Syntax Error: %s. Got ", error_msg);
  dbg_token(peek(parser));
  exit(1);
}

void expect(Parser *parser, TokenType token_type, string error_msg) {
  if (peek(parser).type != token_type) {
    // throw_parser_error(parser, error_msg);
    throw_unexpected_token(parser, error_msg);
  }
}

Token expect_and_eat(Parser *parser, TokenType token_type, string error_msg) {
  Token t = eat(parser);
  if (t.type != token_type) {
    throw_unexpected_token(parser, error_msg);
  }
  return t;
}

Parser new_parser(Token *tokens) {
  Parser p = {.idx = 0, .tokens = tokens};

  return p;
}

Statement *parse_impl_call_stmt(Parser *parser) {

  Token identifier = expect_and_eat(parser, TOKEN_IDENTIFIER,
                                    "expected implementation identifier");

  int arg_len = 0;
  Statement *arg_value = NULL;

  // Argument arg
  if (peek(parser).type == TOKEN_LPAR) {
    eat(parser);
    arg_value = parse_expression(parser);
    // TODO: use optionaly_eat(parser, TOKEN_COMMA);
    expect_and_eat(parser, TOKEN_RPAR,
                   "expected r paren, multiple args not yet supported");
    arg_len += 1;
  }

  return new_impl_call_stmt(LiteralType_i32, identifier.value.str_value,
                            arg_value, identifier);
}

Statement *parse_conditional_stmt(Parser *parser) {
  // printf("\nparsing if staament\n");
  // conditions
  Statement *condition = parse_expression(parser);
  // printf("\ncondition is:\n");
  // dbg_stmt(condition);

  Statement *if_body = parse_block(parser);
  // expect_and_eat(parser, TOKEN_ELSE, "Expected ELSE statement");
  BlockStatement *else_body_block = NULL;
  if (peek(parser).type == TOKEN_ELSE) {
    eat(parser);
    Statement *else_body = parse_block(parser);
    else_body_block = else_body->block;
  }

  // printf("\nbody is:\n");
  // dbg_stmt(if_body);

  return new_conditional_stmt(condition, if_body->block, else_body_block,
                              peek(parser));
}

void expect_one_of(Parser *parser, TokenType *types, int len) {
  Token current_token = peek(parser);
  for (int i = 0; i < len; i++) {
    if (current_token.type == types[i]) {
      return;
    }
  }

  throw_unexpected_token(parser, "Expected one of");
}

Statement *parse_factor(Parser *parser) {
  Token current_token = eat(parser);

  // printf("\n\n %d ? %d \n\n", current_token.type, TOKEN_COLON);
  if (current_token.type == TOKEN_IF) {
    Statement *stmt = parse_conditional_stmt(parser);
    return stmt;
  } else if (current_token.type == TOKEN_IDENTIFIER) {
    return new_var_read_stmt(LiteralType_i32, current_token.value.str_value,
                             current_token);
  } else if (current_token.type == TOKEN_COLON) {
    return parse_impl_call_stmt(parser);
  } else if (current_token.type == TOKEN_NUMBER) {
    return new_i32_literal_stmt(current_token.value.i32_value, current_token);
  } else if (current_token.type == TOKEN_LPAR) {
    // Statement *node = parse_expression(parser);
    Statement *node = parse_statement(parser);
    expect(parser, TOKEN_RPAR, "Expected closing paren");
    eat(parser);
    return node;
  }

  // return NULL;

  // printf("\n CURRENT TOKEN IS");
  // dbg_token(current_token);
  // printf("\n");

  // throw_unexpected_token(
  //     parser, "Unexpected token: Expected Identifier, Number or Open Par");
  return NULL;
}

Statement *parse_term(Parser *parser) {
  Statement *stmt = parse_factor(parser);
  Token current_token = peek(parser);

  while (current_token.type == TOKEN_OP &&
         (current_token.value.op_type == MUL ||
          current_token.value.op_type == DIV)) {

    eat(parser);
    Statement *right = parse_term(parser);
    stmt = new_bin_expr_stmt(current_token.value.op_type, stmt, right,
                             current_token);
    current_token = peek(parser);
  }

  return stmt;
}

Statement *parse_addition(Parser *parser) {

  Statement *stmt = parse_term(parser);

  Token current_token = peek(parser);

  while (current_token.type == TOKEN_OP &&
         (current_token.value.op_type == ADD ||
          current_token.value.op_type == MIN)) {
    eat(parser);
    Statement *right = parse_term(parser);
    stmt = new_bin_expr_stmt(current_token.value.op_type, stmt, right,
                             current_token);
    current_token = peek(parser);
  }

  return stmt;
}

Statement *parse_expression(Parser *parser) {

  Statement *stmt = parse_addition(parser);

  Token current_token = peek(parser);

  while (current_token.type == TOKEN_OP &&
         (current_token.value.op_type == TOKEN_OP_EQ ||
          current_token.value.op_type == TOKEN_OP_GT ||
          current_token.value.op_type == TOKEN_OP_LT ||
          current_token.value.op_type == TOKEN_OP_LTE ||
          current_token.value.op_type == TOKEN_OP_GTE ||
          current_token.value.op_type == TOKEN_OP_BIN_AND ||
          current_token.value.op_type == TOKEN_OP_BIN_OR)) {
    eat(parser);
    Statement *right = parse_addition(parser);
    stmt = new_bin_expr_stmt(current_token.value.op_type, stmt, right,
                             current_token);
    current_token = peek(parser);
  }

  return stmt;
}

Statement *parse_block(Parser *parser) {
  Token current_token = peek(parser);

  if (current_token.type == TOKEN_LBRACKET) {
    eat(parser);
    BlockStatement *block_stmt = new_block_stmt();

    while (current_token.type != TOKEN_RBRACKET) {
      Statement *stmt = parse_statement(parser);
      push_stmt_to_block(stmt, block_stmt);
      current_token = eat(parser);
    }

    Statement *ret_stmt = new_stmt(BLOCK, NULL, NULL, peek(parser));
    ret_stmt->block = block_stmt;
    return ret_stmt;
  }

  return parse_expression(parser);
}

Statement *parse_statement(Parser *parser) {
  Token current_token = peek(parser);

  if (current_token.type == TOKEN_LBRACKET) {
    return parse_block(parser);
  } else if (current_token.type == TOKEN_IMPL) {
    eat(parser);
    Token identifier =
        expect_and_eat(parser, TOKEN_IDENTIFIER, "expected impl identifier");

    expect_and_eat(parser, TOKEN_LPAR,
                   "expected open parenthesis to define arguments");

    // TODO parse multiple args
    Token arg_identifier;
    int arg_len = 0;
    if (peek(parser).type == TOKEN_IDENTIFIER) {
      arg_identifier = eat(parser);
      // printf("declaring arg with name %s", arg_identifier.value.str_value);
      arg_len += 1;
    }

    expect_and_eat(parser, TOKEN_RPAR, "expected closing parenthesis");
    expect(parser, TOKEN_LBRACKET, "expected open bracket to define impl body");

    Statement *block_stmt = parse_statement(parser);
    if (arg_len > 0) {
      block_stmt->block->arg = malloc(sizeof(Argument));
      block_stmt->block->arg->name = arg_identifier.value.str_value;
      block_stmt->block->arg->type = LiteralType_i32;
    }

    return new_impl_decl_stmt(LiteralType_i32, identifier.value.str_value,
                              block_stmt, identifier);

  } else if (current_token.type == TOKEN_LET) {
    eat(parser);
    printf("\n\n");
    dbg_token(peek(parser));
    printf("\n\n");
    expect(parser, TOKEN_IDENTIFIER, "expected identifier");
    Token identifier = eat(parser);

    // dbg_token(identifier);
    expect(parser, TOKEN_EQ, "expected assigment");
    eat(parser);

    Statement *decl =
        new_var_decl_stmt(LiteralType_i32, identifier.value.str_value,
                          parse_expression(parser), identifier);
    return decl;
  } else if (current_token.type == TOKEN_IDENTIFIER) {
    if (look_ahead(parser).type == TOKEN_EQ) {
      Token identifier = eat(parser);
      eat(parser);
      return new_var_write_stmt(LiteralType_i32, identifier.value.str_value,
                                parse_expression(parser), identifier);
    }
  }

  if (current_token.type == TOKEN_PROGRAM_END)
    return NULL;

  // return parse_block(parser);
  return parse_expression(parser);
}

Statement *parse(Parser *parser) {
  Statement *stmt = parse_statement(parser);
  return stmt;
}

#define DEBUG false

BlockStatement *parse_program(Token *tokens) {
  if (DEBUG) {
    printf("\n----\n");
  }
  Parser parser = new_parser(tokens);
  BlockStatement *program = new_block_stmt();

  Statement *stmt = parse(&parser);

  do {
    if (DEBUG) {
      printf("--|STATEMENT|\n");
      dbg_stmt(stmt);
      printf("--|END STATEMENT|\n");
    }

    push_stmt_to_block(stmt, program);
    stmt = parse(&parser);
  } while (stmt != NULL);

  return program;
}
