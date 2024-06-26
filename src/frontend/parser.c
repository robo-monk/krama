#include "parser.h"
#include "LiteralType.h"
#include "Op.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tokeniser.h"
#include <stdio.h>

Statement *parse_statement(Parser *parser);
void parse_symbol_statements(Parser *parser, Vec *syms);

Token look_behind(Parser *parser) { return parser->tokens[parser->idx - 1]; }
Token look_ahead(Parser *parser) { return parser->tokens[parser->idx + 1]; }

Token peek(Parser *parser) { return parser->tokens[parser->idx]; }

Token eat(Parser *parser) { return parser->tokens[parser->idx++]; }

int get_line_number_of_token(Token token) {
  int idx = token.start;
  int line_count = 1;
  for (int i = 0; i <= idx; i++) {
    if (token.tokeniser->content[i] == '\n') {
      line_count++;
    }
  }
  return line_count;
}

string get_substr_range(string str, int start_idx, int end_idx) {
  printf("\n start_idx: %d, end_idx: %d \n", start_idx, end_idx);
  string buffer = calloc(sizeof(char), end_idx - start_idx);
  for (int i = start_idx; i <= end_idx; i++) {
    buffer[i - start_idx] = str[i];
  }
  return buffer;
}

string get_line_of_token(Token token) {
  int idx = token.start;
  int line_count = 1;
  int start_idx = token.start;
  int end_idx = token.end;
  while (token.tokeniser->content[start_idx] != '\n' && start_idx >= 0) {
    start_idx--;
  }
  while (token.tokeniser->content[end_idx] != '\n' &&
         end_idx <= token.tokeniser->char_idx) {
    end_idx++;
  }
  return get_substr_range(token.tokeniser->content, start_idx + 1, end_idx - 1);
}

string line_of_token(Token token) {
  string content = token.tokeniser->content;
  int idx = token.start;

  int start = idx;
  // while (start > 0 && content[start] != '\n') {
  while (start > 0 && content[start] != '\n') {
    start--;
  }
  int end = idx;
  while (content[end] != '\n' && content[end] != '\0') {
    printf("\n%c\n", content[end]);
    end++;
  }
  string line = malloc((end - start) * sizeof(char));
  for (int i = 0; i < (end - start); i++) {
    line[i] = content[start + i];
  }

  int length = token.end - token.start;
  // string pointer = calloc((idx - start) + length, sizeof(char));
  string pointer = calloc(512, sizeof(char));
  // for (int i = 0; i < idx - start - 1; i++) {
  //   pointer[i] = ' ';
  // }
  printf("\ncol_idx: %d\n", token.col_idx);
  for (int i = 0; i < token.col_idx; i++) {
    pointer[i] = ' ';
  }
  for (int i = token.col_idx; i < (token.end - token.start); i++) {
    pointer[i] = '^';
  }
  // return content;
  string line_count_str = malloc(512 * sizeof(char));
  int line_count = get_line_number_of_token(token);
  sprintf(line_count_str, "%d", line_count);
  string line_str = get_line_of_token(token);
  return concat(6, line_count_str, " | ", line_str, "\n", pointer, "\0");
}

void report_syntax_error(Token token, string error_msg) {
  string error_line = line_of_token(token);
  throw_hard_error("%s\n[%d:%d] Syntax error: %s\n", error_line, token.row_idx,
                   token.col_idx, error_msg);
}

void throw_parser_error(Parser *parser, string error_msg) {
  return report_syntax_error(peek(parser), error_msg);
}

void throw_unexpected_token(Parser *parser, string error_msg) {
  return throw_parser_error(parser, error_msg);
}

void expect(Parser *parser, TokenType token_type, string error_msg) {
  if (peek(parser).type == TOKEN_COMMENT) {
    eat(parser);
    return expect(parser, token_type, error_msg);
  }

  if (peek(parser).type != token_type) {
    printf("Got token:");
    dbg_token(peek(parser));
    printf(" - ");
    throw_unexpected_token(parser, error_msg);
  }
}

Token expect_and_eat(Parser *parser, TokenType token_type, string error_msg) {
  expect(parser, token_type, error_msg);
  return eat(parser);
}

Parser new_parser(Token *tokens) {
  Parser p = {.idx = 0, .tokens = tokens};
  return p;
}

void parse_args_until(Parser *parser, BlockStatement *arg_defs,
                      TokenType until) {
  while (peek(parser).type != until) {
    Statement *arg_stmt = parse_expression(parser);

    // printf("-\n");
    // printf("\n parser.c 89 \n");
    push_stmt_to_block(arg_stmt, arg_defs);
    if (peek(parser).type == TOKEN_COMMA) {
      // printf("\nEATING COMMA\n");
      eat(parser);
    }
  }
}

BlockStatement *parse_arg_defs(Parser *parser) {
  BlockStatement *arg_defs = new_block_stmt();
  // Argument arg
  if (peek(parser).type == TOKEN_L_PAR) {
    expect_and_eat(parser, TOKEN_L_PAR, "expected Left paren");
    parse_args_until(parser, arg_defs, TOKEN_R_PAR);
    expect_and_eat(parser, TOKEN_R_PAR, "expected r paren");
  }
  return arg_defs;
}

Statement *parse_impl_call_stmt(Parser *parser) {

  printf("-->current token is \n");
  dbg_token(peek(parser));
  printf("\n");
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

Branch *parse_branch(Parser *parser) {
  expect_and_eat(parser, TOKEN_L_PAR,
                 "expect left bracket to define branch conditions");
  Statement *condition = NULL;

  if (peek(parser).type == TOKEN_R_PAR) {
    eat(parser);
  } else {
    condition = parse_statement(parser);
    expect_and_eat(parser, TOKEN_R_PAR, "Expected closing paren");
  }

  Token arrow = expect_and_eat(parser, TOKEN_RIGHT_ARROW,
                               "expected right arrow to define branch body");

  Statement *body = parse_block(parser);
  return Branch_new(body, condition);
}

Statement *parse_tree_stmt(Parser *parser) {
  Token tree_token =
      expect_and_eat(parser, TOKEN_TREE, "expectd branch statement to eat");

  Token tree_name_token =
      expect_and_eat(parser, TOKEN_IDENTIFIER, "expected tree name");

  expect_and_eat(parser, TOKEN_L_PAR,
                 "expected open parenthesis to define arguments");

  Vec args = new_vec(1, sizeof(SymbolStatement));
  parse_symbol_statements(parser, &args);

  expect_and_eat(parser, TOKEN_R_PAR, "expected close parenthesis");
  expect_and_eat(parser, TOKEN_L_BRACKET,
                 "expectd left bracket to define tree body");

  // BlockStatement *block = new_block_stmt();
  printf("\n bing bong\n");
  Statement *tree_stmt =
      TreeStatement_new(tree_name_token.value.str_value, tree_token, args);

  while (peek(parser).type == TOKEN_L_PAR) {
    Branch *branch = parse_branch(parser);
    TreeStatement_push(tree_stmt->tree, branch);
  }

  expect_and_eat(parser, TOKEN_R_BRACKET,
                 "expectd r bracket to close tree def");

  return tree_stmt;
}

Statement *parse_factor(Parser *parser) {
  Token current_token = peek(parser);

  // printf("\n\n %d ? %d \n\n", current_token.type, TOKEN_COLON);
  if (current_token.type == TOKEN_TREE) {
    printf("\n +++parse branch\n");
    return parse_tree_stmt(parser);
  } else if (current_token.type == TOKEN_IF) {
    eat(parser);
    Statement *stmt = parse_conditional_stmt(parser);
    return stmt;
  } else if (current_token.type == TOKEN_IDENTIFIER) {
    if (look_ahead(parser).type == TOKEN_L_PAR) {
      return parse_impl_call_stmt(parser);
    }
    eat(parser);
    return new_var_read_stmt(LiteralType_i32, current_token.value.str_value,
                             current_token);
  } else if (false && current_token.type == TOKEN_COLON) {

    return parse_impl_call_stmt(parser);
  } else if (current_token.type == TOKEN_NUMBER) {
    eat(parser);
    // return new_i32_literal_stmt(current_token.value.i32_value,
    // current_token);
    return new_numerical_literal_stmt(current_token.value.i32_value,
                                      current_token);
  } else if (current_token.type == TOKEN_CHAR) {
    eat(parser);
    return new_char_literal_stmt(current_token.value.char_value, current_token);
  } else if (current_token.type == TOKEN_L_PAR) {
    eat(parser);

    // Statement *node = parse_expression(parser);
    Statement *node = parse_statement(parser);
    expect(parser, TOKEN_R_PAR, "Expected closing paren");

    eat(parser);
    return node;
  }

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
      eat(parser);

      // Statement *right = parse_term(parser);
      Statement *right = parse_impl_call_stmt(parser);

      stmt = new_bin_expr_stmt(OpType_Custom, stmt, right, current_token);
      current_token = peek(parser);
      // return parse_impl_call_stmt(parser);
      printf("\nafter dbg current token   ");
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

    int i = 0;
    while (true) {
      if (i > 9999) {
        throw_parser_error(parser, "unclosed token r bracket");
      }

      printf("\n ---parsing sttm \n");
      Statement *stmt = parse_statement(parser);

      printf("\n parser.c 289 \n");
      push_stmt_to_block(stmt, block_stmt);
      dbg_stmt(stmt);
      if (peek(parser).type == TOKEN_R_BRACKET) {
        break;
      }
      printf("\n ---parser.c 335 \n");
      i++;
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
  sym_stmt->name = symbol_identifier.value.str_value;

  if (peek(parser).type == TOKEN_COLON) {
    expect_and_eat(parser, TOKEN_COLON, "expected colon to define symbol type");
    Token type_identifier =
        expect_and_eat(parser, TOKEN_IDENTIFIER, "expected type identifier");
    sym_stmt->type = str_to_literal_type(type_identifier.value.str_value);
  } else {
    sym_stmt->type = LiteralType_UNKNOWN;
  }

  if (sym_stmt->type == -1) {
    printf("\n[WARN] Infering type...\n");
    throw_parser_error(parser, "unrecognised literal type");
  }

  return sym_stmt;
}

void parse_symbol_statements(Parser *parser, Vec *syms) {
  int len = 0;
  while (peek(parser).type == TOKEN_IDENTIFIER) {
    vector_push(syms, parse_symbol_statement(parser));
    if (peek(parser).type == TOKEN_COMMA) {
      eat(parser);
    }
  }
}

Statement *parse_definition_stmt(Parser *parser) {
  expect_and_eat(parser, TOKEN_DEF,
                 "parse_def expected current token to be DEF");

  SymbolStatement *namespace = NULL;
  if (peek(parser).type == TOKEN_PIPE) {
    eat(parser);
    namespace = parse_symbol_statement(parser);
    printf("\n|--DEFINE for type %s (defed as: %s) \n",
           literal_type_to_str(namespace->type), namespace->name);

    expect_and_eat(parser, TOKEN_PIPE, "expected closing pipe");
  }

  Token identifier = expect_and_eat(parser, TOKEN_IDENTIFIER,
                                    "expected definition identifier");

  expect_and_eat(parser, TOKEN_L_PAR,
                 "expected open parenthesis to define arguments");

  Vec args = new_vec(1, sizeof(SymbolStatement));
  parse_symbol_statements(parser, &args);

  LiteralType namespace_type = -1;

  // append the namespace in the end
  if (namespace != NULL) {
    vector_push(&args, namespace);
    namespace_type = namespace->type;
  }

  expect_and_eat(parser, TOKEN_R_PAR, "expected closing parenthesis");
  expect(parser, TOKEN_L_BRACKET, "expected open bracket to define impl body");

  Statement *block_stmt = parse_statement(parser);
  Argument **argument_array = (Argument **)vector_alloc_to_array(&args);
  block_stmt->block->args = argument_array;
  block_stmt->block->arg_len = args.size;

  return new_impl_decl_stmt(identifier.value.str_value, block_stmt, identifier);
}

Statement *parse_let(Parser *parser) {
  Token let_token = expect_and_eat(
      parser, TOKEN_LET, "parse_def expected current token to be LET");

  SymbolStatement *sym = parse_symbol_statement(parser);
  expect_and_eat(parser, TOKEN_EQ, "expected assigment");

  Statement *decl = new_var_decl_stmt(sym->type, sym->name,
                                      parse_expression(parser), let_token);
  return decl;
}

Statement *parse_statement(Parser *parser) {
  printf("\n ---parsing sttm  432\n");
  Token current_token = peek(parser);
  printf("\n +++ 434\n");

  if (current_token.type == TOKEN_L_BRACKET) {
    return parse_block(parser);
  } else if (current_token.type == TOKEN_DEF) {
    return parse_definition_stmt(parser);
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
  } else if (current_token.type == TOKEN_COMMENT) {
    eat(parser);
    return new_comment_stmt(current_token);
  }

  if (current_token.type == TOKEN_PROGRAM_END) {
    return NULL;
  }

  // return parse_block(parser);
  return parse_expression(parser);
}

Statement *parse(Parser *parser) {
  Statement *stmt = parse_statement(parser);
  return stmt;
}

#define DEBUG true

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

    printf("\n parser.c 442 \n");
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
