#include "Tokeniser.h"
#include "../utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

Token new_op_token(TokenType type, OpType op_type) {
  Token t = {.type = type, .value = {.op_type = op_type}};
  return t;
}

Token new_number_token(int num) {
  Token t = {.type = NUMBER, .value = {.n = num}};
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
void push_token(Token token, Tokeniser *tokeniser) {
  tokeniser->tokens[tokeniser->idx++] = token;
}

void throw_tokeniser_err(string msg) {
  printf("Tokeniser error: %s", msg);
  exit(1);
}

void push_to_tokeniser_buffer(char c, Tokeniser *tokeniser) {
  if (tokeniser->buffer_idx >= TOKENISER_BUFFER_LEN) {
    throw_tokeniser_err("buffer overflow");
  }
  tokeniser->buffer[tokeniser->buffer_idx++] = c;
}

void commit_buffer_as_number(Tokeniser *tokeniser) {
  int n = str2int(tokeniser->buffer, tokeniser->buffer_idx);
  push_token(new_number_token(n), tokeniser);
}

void commit_buffer_as_string(Tokeniser *tokeniser) {
  while (strcmp("let", tokeniser->buffer) == 0) {
    printf("found let! %s \n", tokeniser->buffer);
    break;
  }
}

void dbg_tokeniser_msg(string msg, Tokeniser *t) {
  printf("Tokeniser [%d]: %s\n", t->idx, msg);
}

void commit_multichar_token(Tokeniser *tokeniser) {
  if (!tokeniser->is_constructing_multichar_token) {
    return;
  }

  MulticharTokenType type;
  // TODO: replace with is_number
  // printf("%s ->", tokeniser->buffer);
  if (is_digit(tokeniser->buffer[0])) {
    type = MULTICHAR_NUM;
    // printf("\nbuffer is [%s]: ", tokeniser->buffer);
    // dbg_tokeniser_msg("found num", tokeniser);
  } else {
    type = MULTICHAR_STR;
    // dbg_tokeniser_msg("found str", tokeniser);
  }

  switch (type) {
  case MULTICHAR_NUM:
    commit_buffer_as_number(tokeniser);
    break;
  case MULTICHAR_STR:
    commit_buffer_as_string(tokeniser);
    break;
  }

  tokeniser->is_constructing_multichar_token = false;
  reset_tokeniser_buffer(tokeniser);
}

void construct_multichar_token(char c, Tokeniser *state) {
  // printf("[%c] ", c);
  // dbg_tokeniser_msg("constructing multichar", state);

  if (!state->is_constructing_multichar_token) {
    state->is_constructing_multichar_token = true;
  }

  push_to_tokeniser_buffer(c, state);
  return;
}

void tokenise_char(char c, Tokeniser *state) {
  switch (c) {
  case '+':
  case '-':
  case '/':
  case '*':
    push_token(new_op_token(OP, c), state);
    break;
  case '(':
  case ')':
    push_token(new_token(c), state);
    break;

  // do not use space as a seperator! 2*2  <-- this breaks
  case ' ':
  case ';':
  case '\n':
    dbg_tokeniser_msg("commiting multichar", state);
    commit_multichar_token(state);
    break;
  default:
    construct_multichar_token(c, state);
    break;
  }
}

Tokeniser *init_tokeniser(Token *tokens) {
  Tokeniser *state = malloc(sizeof(Tokeniser));
  state->idx = 0;
  state->is_constructing_multichar_token = false;
  state->tokens = tokens;
  state->buffer_idx = 0;
  state->buffer = malloc(sizeof(char) * TOKENISER_BUFFER_LEN);
  return state;
}

void close_tokeniser(Tokeniser *tokeniser) {
  commit_multichar_token(tokeniser);
  tokeniser->tokens[tokeniser->idx] = new_token(PROGRAM_END);
  free(tokeniser);
}

void tokenise_str(string str, Token *tokens) {
  Tokeniser *state = init_tokeniser(tokens);

  int str_idx = 0;
  char c;

  while ((c = str[str_idx++]) != '\n') {
    tokenise_char(c, state);
  }

  close_tokeniser(state);
}

void tokenise(string filename, Token *tokens) {
  FILE *fptr;
  fptr = fopen(filename, "r");

  Tokeniser *state = init_tokeniser(tokens);

  char c;
  while ((c = fgetc(fptr)) != EOF) {
    tokenise_char(c, state);
  }

  fclose(fptr);
}

void dbg_token(Token token) {
  if (token.type == OP) {
    printf("Operand: %c", token.value.op_type);
  } else if (token.type == NUMBER) {
    printf("Number: %d", token.value.n);
  } else {
    printf("Token: %c", token.type);
  }
}

void dbg_tokens(Token *tokens) {
  int i = 0;
  while (tokens[i].type != PROGRAM_END) {
    Token current_token = tokens[i++];
    dbg_token(current_token);
    printf("\n");
  }
}
