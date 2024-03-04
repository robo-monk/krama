#ifndef TOKENISER_H
#define TOKENISER_H

#include "../utils.h"

typedef enum { ADD = '+', MIN = '-', DIV = '/', MUL = '*' } OpType;

typedef enum {
  NUMBER,
  OP,
  OPEN_PAR = '(',
  CLOSE_PAR = ')',
  PROGRAM_END
} TokenType;

typedef union {
  int n;
  OpType op_type;
} TokenValue;

typedef struct {
  TokenType type;
  TokenValue value;

  unsigned int start;
  unsigned int end;
  string raw_value;
} Token;

Token new_op_token(TokenType type, OpType op_type);
Token new_token(TokenType type);
void tokenise(string filename, Token *tokens);
void tokenise_str(string filename, Token *tokens);
void dbg_token(Token token);
void dbg_tokens(Token *tokens);

#endif
