#ifndef KRAMA_PARSER_H
#define KRAMA_PARSER_H

#include "ast.h"
#include "tokeniser.h"

typedef struct {
    int index;
    token_t* tokens;
    program_t program;
} parser_t;

typedef enum {
    PRECEDENCE_LOWEST,
    PRECEDENCE_EQUALS, // ==
    PRECEDENCE_LGT, // > or <
    PRECEDENCE_SUM,
    PRECEDENCE_PROD,
    PRECEDENCE_PREFIX,
    PRECEDENCE_CALL
} precedence_t;



void debug_expression(expression_t *expression, int ident);
program_t parse(token_t *tokens);
#endif
