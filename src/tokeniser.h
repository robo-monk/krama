#ifndef TOKENISER_H
#define TOKENISER_H

#include "arena.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    TOKEN_L_BRACE = '{',
    TOKEN_R_BRACE = '}',
    TOKEN_L_BRACKET = '[',
    TOKEN_R_BRACKET = ']',

    TOKEN_L_PAREN = '(',
    TOKEN_R_PAREN = ')',
    TOKEN_COMMA = ',',

    TOKEN_LT = '<',
    TOKEN_GT = '>',

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
    TOKEN_SLASH = '/',
    TOKEN_ASTERISK = '*',
    TOKEN_EQ = '=',

    TOKEN_NEQ = 420,
    TOKEN_EQEQ,
    TOKEN_LTE,
    TOKEN_GTE,
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_ATOM,
    TOKEN_DEFER,
    TOKEN_RETURN,
    TOKEN_FN,
    TOKEN_FOR,
    TOKEN_LOOP,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_UNKNOWN,
    TOKEN_TYPE,
    TOKEN_EXTERN,
    TOKEN_EOF,
} token_type_t;

typedef enum {
    PTYPE_I64,
    PTYPE_I32,
    PTYPE_I16,
    PTYPE_U64,
    PTYPE_U32,
    PTYPE_U16,
    PTYPE_U8,
    PTYPE_F64,
    PTYPE_F32,
    PTYPE_F16,
    PTYPE_CHAR,
    PTYPE_VOID,
    PTYPE_BOOL,
    PTYPE_ANY,
    PTYPE_UNKNOWN=99,
} ptype_t;

typedef union {
    char* raw_str;
    char raw_char;
} token_value_t;

typedef struct {
    token_type_t type;
    int position;
    token_value_t value;
} token_t;

typedef struct {
    char* buffer;
    size_t buffer_index;
    size_t index;
    vector_t tokens;
} tokeniser_t;

int tokenise(const char* data, int data_length, token_t* tokens);
vector_t tokenise2(const char* data, int data_length);
void token_debug(token_t token);
const char* token_type_to_string(token_type_t type);
ptype_t str_to_primitive_type(char* buffer);
const char* primitive_type_to_str(ptype_t t);

#endif
