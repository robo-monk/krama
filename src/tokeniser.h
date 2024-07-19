#ifndef TOKENISER_H
#define TOKENISER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    TOKEN_L_BRACKET = '{',
    TOKEN_R_BRACKET = '}',
    TOKEN_L_PAREN = '(',
    TOKEN_R_PAREN = ')',
    // TOKEN_WHITESPACE = ' ',
    TOKEN_SEMICOLON = ';',

    TOKEN_COLON = ':',
    TOKEN_BANG = '!',
    TOKEN_SINGLE_QUOTE = '\'',
    TOKEN_DOUBLE_QUOTE = '"',
    TOKEN_BACKTICK = '`',
    TOKEN_BACKSLASH = '\\',

    TOKEN_NEW_LINE = '\n',
    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_DIV = '/',
    TOKEN_MULT = '*',
    TOKEN_EQ = '=',
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_DEFER,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} token_type_t;

typedef union {
    char* raw_str;
    char raw_char;
} token_value_t;

typedef struct {
    token_type_t type;
    int position;
    token_value_t value;
} token_t;


int tokenise(const char* data, int data_length, token_t* tokens);
void token_debug(token_t token);

#endif
