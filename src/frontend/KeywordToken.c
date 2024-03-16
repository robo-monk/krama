#include "KeywordToken.h"
#include "stdio.h"
#include "string.h"

TokenType str_to_kw_token(string str) {
  for (int i = 0; i < KW_TOKEN_MAP_SIZE; i++) {
    if (strcmp(kw_token_map[i].str, str) == 0) {
      return kw_token_map[i].token;
    }
  }
  return TOKEN_IDENTIFIER;
}

const string kw_token_to_str(TokenType token) {
  for (int i = 0; i < KW_TOKEN_MAP_SIZE; i++) {
    // if (strcmp(kw_token_map[i].str, str) == 0) {
    if (kw_token_map[i].token == token) {
      return kw_token_map[i].str;
    }
  }
  printf("\n Token is not a keyword !");
  return NULL;
}
