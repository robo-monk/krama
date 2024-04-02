#ifndef TOKENISER_H
#define TOKENISER_H

#include "../utils.h"
#include "Op.h"

typedef enum {
  TOKEN_NUMBER, // number
  TOKEN_OP,     // op
  TOKEN_LET,    // let
  TOKEN_MUT,    // mut
  TOKEN_DEF,    // def
  TOKEN_FN,     // fn
  TOKEN_MT,     // mt
  TOKEN_SHAPE,  // shape
  TOKEN_TARGET, // target
  TOKEN_IDENTIFIER,
  TOKEN_COMMA = ',',
  TOKEN_SINGLE_QUOTE = '\'',
  TOKEN_DOUBLE_QUOTE = '"',
  TOKEN_EQ = '=',
  TOKEN_L_PAR = '(',
  TOKEN_R_PAR = ')',
  TOKEN_L_BRACKET = '{',
  TOKEN_R_BRACKET = '}',
  TOKEN_L_SQ_BRACKET = '[',
  TOKEN_R_SQ_BRACKET = ']',
  TOKEN_COLON = ':',
  TOKEN_PIPE = '|',
  TOKEN_IF = 222, // if
  TOKEN_TREE,
  TOKEN_COMMENT,
  TOKEN_RIGHT_ARROW, // ->
  TOKEN_LEFT_ARROW,  // <-
  TOKEN_ELSE,        // else
  TOKEN_RETURN,      // return
  TOKEN_PROGRAM_END,
} TokenType;

typedef union {
  int i32_value;
  string str_value;
  // TokenOpType op_type;
  OpType op_type;
} TokenValue;

typedef struct Tokeniser Tokeniser;
typedef struct {
  TokenType type;
  TokenValue value;

  unsigned int start;
  unsigned int end;

  int row_idx;
  int col_idx;

  string raw_value;

  const Tokeniser *tokeniser;
} Token;

#define TOKENISER_INITIAL_TOKEN_ARRAY_LEN 100
#define TOKENISER_BUFFER_LEN 512

typedef enum {
  TokeniserBufferType_string,
  TokeniserBufferType_comment,
  TokeniserBufferType_decimal_base10,
} TokeniserBufferType;

typedef struct {
  bool constructing;
  TokeniserBufferType type;
  string str;
  unsigned int idx;
} TokeniserBuffer;

struct Tokeniser {
  string content;
  Token current_token;
  int idx;

  int row_idx;
  int col_idx;

  bool escape;
  Token *tokens;

  TokeniserBuffer buffer;
  // bool is_constructing_multichar_token;
  // string buffer;
  // unsigned int buffer_idx;

  unsigned int char_idx;
  unsigned int _last_char_idx_push;

  unsigned int len;
  unsigned int max_len;
};

Token new_op_token(OpType op_type);
Token new_token(TokenType type);
Tokeniser *tokenise_str(string filename);
void free_tokeniser(Tokeniser *t);
Tokeniser *tokenise_file(const string filename);
void dbg_token(Token token);
void dbg_tokens(Token *tokens);

#endif
