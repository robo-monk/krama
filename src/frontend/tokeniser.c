#include "tokeniser.h"
#include "../utils.h"
#include "Op.h"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tokeniser.h"
#include <ctype.h>

void commit_multichar_token(Tokeniser *tokeniser);
Token new_op_token(OpType op_type) {
  Token t = {.type = TOKEN_OP, .value = {.op_type = op_type}};
  return t;
}

TokeniserBufferType detect_buffer_chartype(char c) {
  if (isdigit(c)) {
    return TokeniserBufferType_decimal_base10;
  } else {
    return TokeniserBufferType_string;
  }
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
  free(tokeniser->buffer.str);
  tokeniser->buffer.str = calloc(TOKENISER_BUFFER_LEN, sizeof(char));
  tokeniser->buffer.idx = 0;
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
  if (tokeniser->buffer.idx >= TOKENISER_BUFFER_LEN) {
    throw_tokeniser_err("Buffer overflow. Multichar token is too big!");
  }

  tokeniser->buffer.str[tokeniser->buffer.idx++] = c;
}

void commit_buffer_as_number(Tokeniser *tokeniser) {
  int n = str2int(tokeniser->buffer.str, tokeniser->buffer.idx);
  push_token(new_number_token(n), tokeniser);
}

typedef struct {
  const string str;
  __MulticharToken token;
} TokenMap;

static const TokenMap token_map[] = {
    {"mut", __MTOKEN_MUT},
    {"let", __MTOKEN_LET},     //
    {"def", __MTOKEN_DEF},     //
    {"shape", __MTOKEN_SHAPE}, //

    {"if", __MTOKEN_IF},         //
    {"else", __MTOKEN_ELSE},     //
    {"return", __MTOKEN_RETURN}, //

    // {"-", __MTOKEN_OP_MIN},       //
    {"->", __MTOKEN_RIGHT_ARROW}, //

    // {"==", __MTOKEN_OP_EQ},  //
    // {"<=", __MTOKEN_OP_LTE}, //
    // {"<", __MTOKEN_OP_LT},   //
    // {">=", __MTOKEN_OP_GTE}, //
    // {">", __MTOKEN_OP_GT},   //
    {NULL, 0} //
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

  OpType op = str_to_optype(tokeniser->buffer.str);
  if (op != -1) {
    return push_token(new_op_token(op), tokeniser);
  }

  __MulticharToken mulitchar_token =
      __map_string_to_multichar_token(tokeniser->buffer.str);

  switch (mulitchar_token) {
  case __MTOKEN_IDENTIFIER:
    return push_token(new_str_token(TOKEN_IDENTIFIER, tokeniser->buffer.str,
                                    tokeniser->buffer.idx),
                      tokeniser);
  case __MTOKEN_LET:
    return push_token(new_token(TOKEN_LET), tokeniser);
  case __MTOKEN_DEF:
    return push_token(new_token(TOKEN_DEF), tokeniser);
  case __MTOKEN_SHAPE:
    return push_token(new_token(TOKEN_SHAPE), tokeniser);
  case __MTOKEN_MUT:
    return push_token(new_token(TOKEN_MUT), tokeniser);
  case __MTOKEN_ELSE:
    return push_token(new_token(TOKEN_ELSE), tokeniser);
  case __MTOKEN_RETURN:
    return push_token(new_token(TOKEN_RETURN), tokeniser);
  case __MTOKEN_RIGHT_ARROW:
    return push_token(new_token(TOKEN_RIGHT_ARROW), tokeniser);
  // case __MTOKEN_OP_MIN:
  //   return push_token(new_op_token(OpType_SUB), tokeniser);
  // case __MTOKEN_OP_EQ:
  //   return push_token(new_op_token(OpType_EQ), tokeniser);
  // case __MTOKEN_OP_GTE:
  //   return push_token(new_op_token(OpType_GTE), tokeniser);
  // case __MTOKEN_OP_LTE:
  //   return push_token(new_op_token(OpType_LTE), tokeniser);
  // case __MTOKEN_OP_GT:
  //   return push_token(new_op_token(OpType_GT), tokeniser);
  // case __MTOKEN_OP_LT:
  //   return push_token(new_op_token(OpType_LT), tokeniser);
  case __MTOKEN_IF:
    return push_token(new_token(TOKEN_IF), tokeniser);
  }

  throw_tokeniser_err("unreachable?");
}

void commit_multichar_token(Tokeniser *tokeniser) {
  if (!tokeniser->buffer.constructing) {
    return;
  }

  if (is_digit(tokeniser->buffer.str[0])) {
    commit_buffer_as_number(tokeniser);
  } else {
    commit_buffer_as_string(tokeniser);
  }

  tokeniser->buffer.constructing = false;
  reset_tokeniser_buffer(tokeniser);
}

void construct_multichar_token(char c, Tokeniser *state) {
  if (!state->buffer.constructing) {
    state->buffer.constructing = true;
    state->buffer.type = detect_buffer_chartype(c);
  } else if (state->buffer.type != TokeniserBufferType_string &&
             state->buffer.type != detect_buffer_chartype(c)) {
    // printf("detected change in types. commitng buffer");
    commit_multichar_token(state);
    return construct_multichar_token(c, state);
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
    construct_multichar_token(c, state);
    commit_multichar_token(state);
    break;
  case '=':
    commit_multichar_token(state);
    push_token(new_token(TOKEN_EQ), state);
    break;
  case ',':
  case '(':
  case ')':
  case '[':
  case ']':
  case '{':
  case '}':
  case ':':
  case '|':
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

  state->char_idx = 0;
  state->_last_char_idx_push = 0;

  state->tokens = malloc(sizeof(Token) * TOKENISER_INITIAL_TOKEN_ARRAY_LEN);
  state->len = 0;
  state->max_len = TOKENISER_INITIAL_TOKEN_ARRAY_LEN;

  state->buffer =
      (TokeniserBuffer){.idx = 0,
                        .constructing = false,
                        .str = calloc(TOKENISER_BUFFER_LEN, sizeof(char))};

  return state;
}

void close_tokeniser(Tokeniser *tokeniser) {
  commit_multichar_token(tokeniser);
  tokeniser->tokens[tokeniser->idx] = new_token(TOKEN_PROGRAM_END);
}

void free_tokeniser(Tokeniser *t) {
  free(t->buffer.str);
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
  printf("tokend");
  dbg_tokens(tokeniser->tokens);
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
  return tokeniser;
}

void dbg_token(Token token) {
  printf("[%d:%d] ", token.row_idx + 1, token.col_idx + 1);
  switch (token.type) {
  case TOKEN_OP:
    printf("Operand: %s", optype_to_str(token.value.op_type));
    break;
  case TOKEN_NUMBER:
    printf("Number: %d", token.value.i32_value);
    break;
  case TOKEN_IDENTIFIER:
    printf("Identifier: %s", token.value.str_value);
    break;
  case TOKEN_DEF:
    printf("Implementation");
    break;
  case TOKEN_LET:
    printf("Token: LET");
    break;
  case TOKEN_MUT:
    printf("Token: MUT");
    break;
  case TOKEN_SHAPE:
    printf("Token: SHAPE");
    break;
  case TOKEN_RETURN:
    printf("RETURN");
    break;
  case TOKEN_ELSE:
    printf("Else statement");
    break;
  case TOKEN_R_BRACKET:
  case TOKEN_L_BRACKET:
  case TOKEN_L_PAR:
  case TOKEN_R_PAR:
  case TOKEN_L_SQ_BRACKET:
  case TOKEN_R_SQ_BRACKET:
  case TOKEN_EQ:
  case TOKEN_COLON:
  case TOKEN_PIPE:
  case TOKEN_COMMA:
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
