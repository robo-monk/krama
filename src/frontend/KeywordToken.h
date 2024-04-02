#ifndef KWTOKEN_H
#define KWTOKEN_H
#include "../utils.h"
#include "tokeniser.h"

// typedef enum {
//   KW_TOKEN_LET,
//   KW_TOKEN_MUT,
//   KW_TOKEN_DEF,
//   KW_TOKEN_SHAPE,
//   KW_TOKEN_IF,
//   KW_TOKEN_ELSE,
//   KW_TOKEN_IDENTIFIER,
//   KW_TOKEN_RETURN,
//   KW_TOKEN_RIGHT_ARROW
// } KeywordToken;

typedef struct {
  const string str;
  // KeywordToken token;
  TokenType token;
} KeywordTokenMap;

static const KeywordTokenMap kw_token_map[] = {
    {"mut", TOKEN_MUT},       //
    {"let", TOKEN_LET},       //
    {"def", TOKEN_DEF},       //
    {"mt", TOKEN_MT},         //
    {"fn", TOKEN_FN},         //
    {"target", TOKEN_TARGET}, //
    {"shape", TOKEN_SHAPE},   //

    {"if", TOKEN_IF},         //
    {"tree", TOKEN_TREE},     //
    {"else", TOKEN_ELSE},     //
    {"return", TOKEN_RETURN}, //

    {"->", TOKEN_RIGHT_ARROW}, //
    {"<-", TOKEN_LEFT_ARROW},  //
};

#define KW_TOKEN_MAP_SIZE (sizeof(kw_token_map) / sizeof(KeywordTokenMap))

TokenType str_to_kw_token(string str);
const string kw_token_to_str(TokenType token);

#endif
