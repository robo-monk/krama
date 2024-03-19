#include "parser.h"
#include "LiteralType.h"
#include "Op.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tokeniser.h"

Statement *parse_statement(Parser *parser);

Token look_behind(Parser *parser) { return parser->tokens[parser->idx - 1]; }
Token look_ahead(Parser *parser) { return parser->tokens[parser->idx + 1]; }

Token peek(Parser *parser) { return parser->tokens[parser->idx]; }

Token eat(Parser *parser) { return parser->tokens[parser->idx++]; }

string line_of_token(Token token) {
  string content = token.tokeniser->content;
  int idx = token.start;

  int start = idx;
  while (start > 0 && content[start] != '\n') {
    start--;
  }
  int end = idx;
  while (content[end] != '\n' && content[end] != '\0') {
    end++;
  }
  string line = malloc((end - start) * sizeof(char));
  for (int i = 0; i < (end - start); i++) {
    line[i] = content[start + i];
  }

  int length = token.end - token.start;
  string pointer = calloc((idx - start) + length, sizeof(char));
  for (int i = 0; i < idx - start - 1; i++) {
    pointer[i] = ' ';
  }
  for (int i = idx - start - 1; i < idx - start + length - 2; i++) {
    pointer[i] = '^';
  }
  return concat(4, line, "\n", pointer, "\0");
}

void report_syntax_error(Token token, string error_msg) {
  string error_line = line_of_token(token);
  throw_hard_error("%s\n[%d:%d] Syntax error: %s\n", error_line, token.row_idx,
                   token.col_idx, error_msg);
}

void throw_parser_error(Parser *parser, string error_msg) {
  return report_syntax_error(look_behind(parser), error_msg);
}

void throw_unexpected_token(Parser *parser, string error_msg) {
  return throw_parser_error(parser, error_msg);
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

BlockStatement *parse_arg_defs(Parser *parser) {

  BlockStatement *arg_defs = new_block_stmt();
  // Argument arg
  if (peek(parser).type == TOKEN_L_PAR) {
    expect_and_eat(parser, TOKEN_L_PAR, "expected Left paren");
    do {
      Statement *arg_stmt = parse_expression(parser);
      dbg_stmt(arg_stmt);
      push_stmt_to_block(arg_stmt, arg_defs);
      if (peek(parser).type == TOKEN_COMMA) {
        printf("\nEATING COMMA\n");
        eat(parser);
      }
    } while (peek(parser).type != TOKEN_R_PAR);

    expect_and_eat(parser, TOKEN_R_PAR, "expected r paren");
  }
  return arg_defs;
}

Statement *parse_impl_call_stmt(Parser *parser) {

  Token identifier = expect_and_eat(parser, TOKEN_IDENTIFIER,
                                    "expected implementation identifier");

  return new_impl_call_stmt(LiteralType_i32, identifier.value.str_value,
                            parse_arg_defs(parser), identifier);
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
  } else if (current_token.type == TOKEN_L_PAR) {
    // Statement *node = parse_expression(parser);
    Statement *node = parse_statement(parser);
    expect(parser, TOKEN_R_PAR, "Expected closing paren");
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
             (current_token.value.op_type == OpType_MUL ||
              current_token.value.op_type == OpType_MOD ||
              current_token.value.op_type == OpType_DIV) ||
         current_token.type == TOKEN_COLON) {

    if (current_token.type == TOKEN_COLON) {
      printf("\ndbg current token   ");
      dbg_token(current_token);
      printf("\n");
      // eat(parser);
      Statement *right = parse_term(parser);
      stmt = new_bin_expr_stmt(OpType_Custom, stmt, right, current_token);
      current_token = peek(parser);
      printf("\ndbg current token   ");
      dbg_token(current_token);
      printf("\n");

      continue;
    }

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
         (current_token.value.op_type == OpType_ADD ||
          current_token.value.op_type == OpType_SUB)) {
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
         (current_token.value.op_type == OpType_EQ ||
          current_token.value.op_type == OpType_GT ||
          current_token.value.op_type == OpType_LT ||
          current_token.value.op_type == OpType_LTE ||
          current_token.value.op_type == OpType_GTE ||
          current_token.value.op_type == OpType_BIN_AND ||
          current_token.value.op_type == OpType_BIN_OR)) {
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

  if (current_token.type == TOKEN_L_BRACKET) {
    eat(parser);
    BlockStatement *block_stmt = new_block_stmt();

    while (true) {
      Statement *stmt = parse_statement(parser);
      push_stmt_to_block(stmt, block_stmt);
      dbg_stmt(stmt);
      if (peek(parser).type == TOKEN_R_BRACKET) {
        break;
      }
    }
    expect_and_eat(parser, TOKEN_R_BRACKET, "Expected r bracket to eat");

    Statement *ret_stmt = new_stmt(STMT_BLOCK, NULL, NULL, peek(parser));
    ret_stmt->block = block_stmt;
    return ret_stmt;
  }

  return parse_expression(parser);
}

SymbolStatement *parse_symbol_statement(Parser *parser) {
  Token symbol_identifier =
      expect_and_eat(parser, TOKEN_IDENTIFIER,
                     "parse_symbol_statement expected symbol identifier");
  SymbolStatement *sym_stmt = malloc(sizeof(SymbolStatement));

  expect_and_eat(parser, TOKEN_COLON, "expected colon to define symbol type");

  Token type_identifier =
      expect_and_eat(parser, TOKEN_IDENTIFIER, "expected type identifier");

  sym_stmt->name = symbol_identifier.value.str_value;
  sym_stmt->type = str_to_literal_type(type_identifier.value.str_value);
  if (sym_stmt->type == -1) {
    throw_parser_error(parser, "unrecognised literal type");
  }
  return sym_stmt;
}

void parse_symbol_statements(Parser *parser, Vec *syms) {
  int len = 0;
  while (peek(parser).type == TOKEN_IDENTIFIER) {
    vector_push(syms, parse_symbol_statement(parser));
    // syms[len] = parse_symbol_statement(parser);
    // len += 1;
    if (peek(parser).type == TOKEN_COMMA) {
      eat(parser);
    }
  }
  // return len;
}

Statement *parse_def(Parser *parser) {
  expect_and_eat(parser, TOKEN_DEF,
                 "parse_def expected current token to be DEF");

  SymbolStatement *namespace = NULL;
  if (peek(parser).type == TOKEN_PIPE) {
    eat(parser);
    // throw_parser_error(parser, "not implemented");
    namespace = parse_symbol_statement(parser);
    printf("\n|--DEFINE for type %s (defed as: %s) \n",
           literal_type_to_str(namespace->type), namespace->name);

    expect_and_eat(parser, TOKEN_PIPE, "expected closing pipe");
  }

  Token identifier = expect_and_eat(parser, TOKEN_IDENTIFIER,
                                    "expected definition identifier");

  expect_and_eat(parser, TOKEN_L_PAR,
                 "expected open parenthesis to define arguments");

  // TODO: this should be a dynamic array
  // SymbolStatement **args = malloc(sizeof(SymbolStatement) * 100);
  Vec args = new_vec(1, sizeof(SymbolStatement));

  parse_symbol_statements(parser, &args);

  // append the namespace in the end
  if (namespace != NULL) {
    vector_push(&args, namespace);
  }

  expect_and_eat(parser, TOKEN_R_PAR, "expected closing parenthesis");
  expect(parser, TOKEN_L_BRACKET, "expected open bracket to define impl body");

  Statement *block_stmt = parse_statement(parser);
  block_stmt->block->args = calloc(args.size, sizeof(SymbolStatement));

  for (int i = 0; i < args.size; i++) {
    block_stmt->block->args[i] = vector_at(&args, i);
  }

  block_stmt->block->arg_len = args.size;

  LiteralType namespace_type = -1;

  if (namespace != NULL) {
    namespace_type = namespace->type;
  }

  return new_impl_decl_stmt(namespace_type, identifier.value.str_value,
                            block_stmt, identifier);
}

Statement *parse_let(Parser *parser) {
  expect_and_eat(parser, TOKEN_LET,
                 "parse_def expected current token to be LET");
  expect(parser, TOKEN_IDENTIFIER, "expected identifier");
  Token identifier = eat(parser);

  // dbg_token(identifier);
  expect(parser, TOKEN_EQ, "expected assigment");
  eat(parser);

  Statement *decl =
      new_var_decl_stmt(LiteralType_i32, identifier.value.str_value,
                        parse_expression(parser), identifier);
  return decl;
}

Statement *parse_statement(Parser *parser) {
  Token current_token = peek(parser);

  if (current_token.type == TOKEN_L_BRACKET) {
    return parse_block(parser);
  } else if (current_token.type == TOKEN_DEF) {
    return parse_def(parser);
  } else if (current_token.type == TOKEN_LET) {
    return parse_let(parser);
  } else if (current_token.type == TOKEN_IDENTIFIER) {
    if (look_ahead(parser).type == TOKEN_EQ) {
      Token identifier = eat(parser);
      expect_and_eat(parser, TOKEN_EQ,
                     "expected equal sign to assing value to variable");
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

BlockStatement *parse_tokens(Token *tokens) {
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

BlockStatement *program_from_tokeniser(Tokeniser *t) {
  return parse_tokens(t->tokens);
}

BlockStatement *program_from_filename(const string filename) {
  return program_from_tokeniser(tokenise_file(filename));
}
