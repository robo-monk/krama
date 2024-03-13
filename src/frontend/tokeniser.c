#include "Tokeniser.h"
#include "../utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

Token new_op_token(OpType op_type) {
  Token t = {.type = TOKEN_OP, .value = {.op_type = op_type}};
  return t;
}

Token new_str_token(TokenType type, string value, int len) {
  Token t = {.type = type};
  t.value.str_value = malloc(len * sizeof(char));
  for (int i = 0; i < len; i++) {
    t.value.str_value[i] = value[i];
  }
  return t;
}

Token new_number_token(int num) {
  Token t = {.type = TOKEN_NUMBER, .value = {.i32_value = num}};
  return t;
}

Token new_token(TokenType type) {
  Token t = {.type = type};
  return t;
}

void reset_tokeniser_buffer(Tokeniser *tokeniser) {
  free(tokeniser->buffer);
  tokeniser->buffer = malloc(sizeof(char) * TOKENISER_BUFFER_LEN);
  tokeniser->buffer_idx = 0;
}

void _alloc_tokens_array(Tokeniser *tokeniser) {
  if (tokeniser->len < tokeniser->max_len) {
    return;
  }

  printf("\nALLOC MORE MEM\n");
  tokeniser->max_len = tokeniser->max_len * 2;

  Token *new_mem =
      realloc(tokeniser->tokens, sizeof(Token) * tokeniser->max_len);

  if (new_mem == NULL) {
    printf("faield to alloc memoery");
    // throw_tokeniser_err("failed to alloc memory");
    return;
  }

  tokeniser->tokens = new_mem;
}
void push_token(Token token, Tokeniser *tokeniser) {
  token.start = tokeniser->_last_char_idx_push;
  token.end = tokeniser->char_idx;
  token.row_idx = tokeniser->row_idx;
  token.col_idx = tokeniser->col_idx;

  tokeniser->len += 1;
  _alloc_tokens_array(tokeniser);

  tokeniser->tokens[tokeniser->idx++] = token;
  tokeniser->_last_char_idx_push = tokeniser->char_idx;
  tokeniser->col_idx++;
}

void dbg_tokeniser_msg(string msg, Tokeniser *t) {
  printf("Tokeniser [%d]: %s\n", t->idx, msg);
}
void throw_tokeniser_err(string msg) {
  printf("Tokeniser error: %s", msg);
  exit(1);
}

void push_to_tokeniser_buffer(char c, Tokeniser *tokeniser) {
  if (tokeniser->buffer_idx >= TOKENISER_BUFFER_LEN) {
    throw_tokeniser_err("Buffer overflow. Multichar token is too big!");
  }
  tokeniser->buffer[tokeniser->buffer_idx++] = c;
}

void commit_buffer_as_number(Tokeniser *tokeniser) {
  int n = str2int(tokeniser->buffer, tokeniser->buffer_idx);
  push_token(new_number_token(n), tokeniser);
}

typedef struct {
  const string str;
  __MulticharToken token;
} TokenMap;

static const TokenMap token_map[] = {
    {"let", __MTOKEN_LET},       //
    {"fn", __MTOKEN_IMPL},       //
    {"if", __MTOKEN_IF},         //
    {"else", __MTOKEN_ELSE},     //
    {"return", __MTOKEN_RETURN}, //
    {"==", __MTOKEN_OP_EQ},      //
    {"<=", __MTOKEN_OP_LTE},     //
    {"<", __MTOKEN_OP_LT},       //
    {">=", __MTOKEN_OP_GTE},     //
    {">", __MTOKEN_OP_GT},       //
    {NULL, 0}                    //
};

__MulticharToken __map_string_to_multichar_token(string str) {
  for (const TokenMap *map = token_map; map->str != NULL; ++map) {
    if (strcmp(map->str, str) == 0) {
      return map->token;
    }
  }
  return __MTOKEN_IDENTIFIER;
}

void commit_buffer_as_string(Tokeniser *tokeniser) {
  __MulticharToken mulitchar_token =
      __map_string_to_multichar_token(tokeniser->buffer);

  switch (mulitchar_token) {
  case __MTOKEN_IDENTIFIER:
    break;
  case __MTOKEN_LET:
    return push_token(new_token(TOKEN_LET), tokeniser);
  case __MTOKEN_IMPL:
    return push_token(new_token(TOKEN_IMPL), tokeniser);
  case __MTOKEN_ELSE:
    return push_token(new_token(TOKEN_ELSE), tokeniser);
  case __MTOKEN_RETURN:
    return push_token(new_token(TOKEN_RETURN), tokeniser);
  case __MTOKEN_OP_EQ:
    return push_token(new_op_token(TOKEN_OP_EQ), tokeniser);
  case __MTOKEN_OP_GTE:
    return push_token(new_op_token(TOKEN_OP_GTE), tokeniser);
  case __MTOKEN_OP_LTE:
    return push_token(new_op_token(TOKEN_OP_LTE), tokeniser);
  case __MTOKEN_OP_GT:
    return push_token(new_op_token(TOKEN_OP_GT), tokeniser);
  case __MTOKEN_OP_LT:
    return push_token(new_op_token(TOKEN_OP_LT), tokeniser);
  case __MTOKEN_IF:
    return push_token(new_token(TOKEN_IF), tokeniser);
  }

  return push_token(
      new_str_token(TOKEN_IDENTIFIER, tokeniser->buffer, tokeniser->buffer_idx),
      tokeniser);
}

void commit_multichar_token(Tokeniser *tokeniser) {
  if (!tokeniser->is_constructing_multichar_token) {
    return;
  }

  if (is_digit(tokeniser->buffer[0])) {
    commit_buffer_as_number(tokeniser);
  } else {
    commit_buffer_as_string(tokeniser);
  }

  tokeniser->is_constructing_multichar_token = false;
  reset_tokeniser_buffer(tokeniser);
}

void construct_multichar_token(char c, Tokeniser *state) {
  if (!state->is_constructing_multichar_token) {
    state->is_constructing_multichar_token = true;
  }

  push_to_tokeniser_buffer(c, state);
}

void tokenise_char(char c, Tokeniser *state) {
  switch (c) {
  case '+':
  case '-':
  case '/':
  case '*':
    commit_multichar_token(state);
    push_token(new_op_token(c), state);
    break;
  case '=':
    commit_multichar_token(state);
    push_token(new_token(TOKEN_EQ), state);
    break;
  case '(':
  case ')':
  case '{':
  case '}':
  case ':':
    commit_multichar_token(state);
    push_token(new_token(c), state);
    break;

  case '\r':
  case '\n':
    state->row_idx += 1;
    state->col_idx = 0;
  case '\t':
  case '\v':
  case '\f':
  case ' ':
  case ';':
    // dbg_tokeniser_msg("commiting multichar", state);
    commit_multichar_token(state);
    break;
  default:
    construct_multichar_token(c, state);
    break;
  }
}

Tokeniser *new_tokeniser() {
  Tokeniser *state = malloc(sizeof(Tokeniser));
  state->idx = 0;

  state->row_idx = 0;
  state->col_idx = 0;

  state->is_constructing_multichar_token = false;
  state->char_idx = 0;
  state->_last_char_idx_push = 0;

  state->tokens = malloc(sizeof(Token) * TOKENISER_INITIAL_TOKEN_ARRAY_LEN);
  state->len = 0;
  state->max_len = TOKENISER_INITIAL_TOKEN_ARRAY_LEN;

  state->buffer_idx = 0;
  state->buffer = malloc(sizeof(char) * TOKENISER_BUFFER_LEN);
  return state;
}

void close_tokeniser(Tokeniser *tokeniser) {
  commit_multichar_token(tokeniser);
  tokeniser->tokens[tokeniser->idx] = new_token(TOKEN_PROGRAM_END);
}

void free_tokeniser(Tokeniser *t) {
  free(t->buffer);
  free(t->tokens);
  free(t);
}

Tokeniser *tokenise_str(string str) {
  Tokeniser *tokeniser = new_tokeniser();
  char c;

  while ((c = str[tokeniser->char_idx++]) != '\n') {
    tokenise_char(c, tokeniser);
  }

  close_tokeniser(tokeniser);
  return tokeniser;
}

Tokeniser *tokenise_file(const string filename) {
  FILE *fptr;
  fptr = fopen(filename, "r");

  Tokeniser *tokeniser = new_tokeniser();

  char c;
  while ((c = fgetc(fptr)) != EOF) {
    // printf("\nconsum %c", c);
    tokenise_char(c, tokeniser);
  }
  close_tokeniser(tokeniser);

  fclose(fptr);
}

void dbg_token(Token token) {
  printf("[%d:%d] ", token.row_idx + 1, token.col_idx);
  switch (token.type) {
  case TOKEN_OP:
    printf("Operand: %c", token.value.op_type);
    break;
  case TOKEN_NUMBER:
    printf("Number: %d", token.value.i32_value);
    break;
  case TOKEN_IDENTIFIER:
    printf("Identifier: %s", token.value.str_value);
    break;
  case TOKEN_IMPL:
    printf("Implementation");
    break;
  case TOKEN_LET:
    printf("Token: LET");
    break;
  case TOKEN_RETURN:
    printf("RETURN");
    break;
  case TOKEN_ELSE:
    printf("Else statement");
    break;
  case TOKEN_RBRACKET:
  case TOKEN_LBRACKET:
  case TOKEN_LPAR:
  case TOKEN_RPAR:
  case TOKEN_EQ:
  case TOKEN_COLON:
    printf("Token: '%c'", token.type);
    break;
  case TOKEN_PROGRAM_END:
    printf("PROGRAM_END");
    break;
  case TOKEN_IF:
    printf("If Statement");
  }
}

void dbg_tokens(Token *tokens) {
  int i = 0;
  while (tokens[i].type != TOKEN_PROGRAM_END) {
    Token current_token = tokens[i++];
    dbg_token(current_token);
    printf("\n");
  }
}
