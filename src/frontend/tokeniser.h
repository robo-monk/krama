#ifndef TOKENISER_H
#define TOKENISER_H

#include "../utils.h"

typedef enum {
  __MTOKEN_OP_EQ,
  __MTOKEN_OP_LTE,
  __MTOKEN_OP_GTE,
  __MTOKEN_LET,
  __MTOKEN_IMPL,
  __MTOKEN_IF,
} __MulticharToken;

typedef enum {
  ADD = '+',
  MIN = '-',
  DIV = '/',
  MUL = '*',
  LT = '<',
  GT = '>',
  MODULUS = '%',
  BIN_AND = '&',
  BIN_OR = '|',
  BIN_XOR = '^',

  TOKEN_OP_EQ,
  TOKEN_OP_LTE,
  TOKEN_OP_GTE
} OpType;

typedef enum {
  TOKEN_NUMBER,
  TOKEN_OP,
  TOKEN_LET,
  TOKEN_IMPL,
  TOKEN_IDENTIFIER,
  TOKEN_EQ = '=',
  TOKEN_LPAR = '(',
  TOKEN_RPAR = ')',
  TOKEN_LBRACKET = '{',
  TOKEN_RBRACKET = '}',
  TOKEN_COLON = ':',
  TOKEN_IF,
  TOKEN_PROGRAM_END,
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
void tokenise_str(string filename, Token *tokens);
void tokenise_file(const string filename, Token *tokens);
void dbg_token(Token token);
void dbg_tokens(Token *tokens);

#endif
