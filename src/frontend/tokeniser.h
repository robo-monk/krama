#ifndef TOKENISER_H
#define TOKENISER_H

#include "../utils.h"

typedef enum {
  __MTOKEN_OP_EQ,
  __MTOKEN_OP_LT,
  __MTOKEN_OP_LTE,
  __MTOKEN_OP_GT,
  __MTOKEN_OP_GTE,
  __MTOKEN_LET,
  __MTOKEN_IMPL,
  __MTOKEN_IF,
  __MTOKEN_ELSE,
  __MTOKEN_IDENTIFIER,
  __MTOKEN_RETURN
} __MulticharToken;

typedef enum {
  ADD = '+',
  MIN = '-',
  DIV = '/',
  MUL = '*',

  MODULUS = '%',
  TOKEN_BITWISE_AND = '&',
  TOKEN_BITWISE_OR = '|',
  TOKEN_BITWISE_XOR = '^',

  TOKEN_OP_LT = '<',
  TOKEN_OP_GT = '>',
  TOKEN_OP_EQ,
  TOKEN_OP_LTE,
  TOKEN_OP_GTE,
  TOKEN_OP_BIN_AND,
  TOKEN_OP_BIN_OR,
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
  TOKEN_IF = 222,
  TOKEN_ELSE,
  TOKEN_RETURN,
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

  int row_idx;
  int col_idx;

  string raw_value;
} Token;

#define TOKENISER_BUFFER_LEN 512

typedef struct {
  Token current_token;
  int idx;

  int row_idx;
  int col_idx;

  Token *tokens;

  bool is_constructing_multichar_token;
  string buffer;
  unsigned int buffer_idx;

  unsigned int char_idx;
  unsigned int _last_char_idx_push;
} Tokeniser;

Token new_op_token(OpType op_type);
Token new_token(TokenType type);
void tokenise_str(string filename, Token *tokens);
void tokenise_file(const string filename, Token *tokens);
void dbg_token(Token token);
void dbg_tokens(Token *tokens);

#endif
