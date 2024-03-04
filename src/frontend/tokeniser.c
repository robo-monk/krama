#include "Tokeniser.h"
#include "../utils.h"
#include "stdio.h"
#include "stdlib.h"

Token new_op_token(TokenType type, OpType op_type) {
  Token t = {.type = type, .value = {.op_type = op_type}};
  return t;
}

Token new_token(TokenType type) {
  Token t = {.type = type};
  return t;
}

int parse_number(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  return -1;
}

typedef struct {
  bool is_constructing_multichar_token;
  Token current_token;
  int idx;
  Token *tokens;
} TokeniserState;

void tokenise_char(char c, TokeniserState *state) {
  int number = parse_number(c);

  if (number > -1) {
    if (!state->is_constructing_multichar_token) {
      state->is_constructing_multichar_token = true;
      state->current_token = new_token(NUMBER);
      state->current_token.value.n = number;
    } else {
      state->current_token.value.n =
          (state->current_token.value.n * 10) + number;
    }
    return;
  }

  // done with construction
  if (state->is_constructing_multichar_token) {
    state->tokens[state->idx++] = state->current_token;
    state->is_constructing_multichar_token = false;
  }

  switch (c) {
  case '+':
  case '-':
  case '/':
  case '*':
    state->tokens[state->idx++] = new_op_token(OP, c);
    break;
  case '(':
  case ')':
    state->tokens[state->idx++] = new_token(c);
    break;
  case '$':
    state->tokens[state->idx++] = new_token(c);
    break;
  case ' ':
    break;
  default:
    printf("unrecognised char");
    exit(1);
    break;
  }
}

TokeniserState *init_tokeniser(Token *tokens) {
  TokeniserState *state = malloc(sizeof(TokeniserState));
  state->idx = 0;
  state->is_constructing_multichar_token = false;
  state->tokens = tokens;
  return state;
}

void close_tokeniser(TokeniserState *state) {
  if (state->is_constructing_multichar_token) {
    state->tokens[state->idx++] = state->current_token;
  }

  state->tokens[state->idx] = new_token(PROGRAM_END);
  free(state);
}

void tokenise_str(string str, Token *tokens) {
  TokeniserState *state = init_tokeniser(tokens);

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

  TokeniserState *state = init_tokeniser(tokens);

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
