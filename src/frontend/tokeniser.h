#ifndef TOKENISER_H
#define TOKENISER_H

#include "../utils.h"

typedef enum { ADD = '+', MIN = '-', DIV = '/', MUL = '*' } OpType;

typedef enum {
  NUMBER,
  OP,
  LET,
  IDENTIFIER,
  EQ = '=',
  OPEN_PAR = '(',
  CLOSE_PAR = ')',
  PROGRAM_END
} TokenType;

typedef union {
  int i32_value;
  string str_value;
  OpType op_type;
} TokenValue;

typedef struct {
  TokenType type;
  TokenValue value;

  unsigned int start;
  unsigned int end;
  string raw_value;
} Token;

typedef enum {
  MULTICHAR_STR,
  MULTICHAR_NUM,
} MulticharTokenType;

#define TOKENISER_BUFFER_LEN 512

typedef struct {
  Token current_token;
  int idx;
  Token *tokens;

  bool is_constructing_multichar_token;
  string buffer;
  unsigned int buffer_idx;

  unsigned int char_idx;
  unsigned int _last_char_idx_push;
} Tokeniser;

Token new_op_token(TokenType type, OpType op_type);
Token new_token(TokenType type);
void tokenise(string filename, Token *tokens);
void tokenise_str(string filename, Token *tokens);
void dbg_token(Token token);
void dbg_tokens(Token *tokens);

#endif
