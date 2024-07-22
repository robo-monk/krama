#ifndef KRAMA_PARSER_H
#define KRAMA_PARSER_H

#include "ast.h"
#include "tokeniser.h"
#include "hashmap.h"

typedef struct scope_t {
    hashmap_t *table;
    struct scope_t *upper;
} scope_t;

typedef struct {
    identifier_expression_t identifier;
} scope_entry_t;

typedef struct {
    char* message;
    token_t token;
} parser_error_t;

#define PARSER_MAX_ERROR_COUNT 8
typedef struct {
    int index;
    token_t* tokens;
    program_t program;
    parser_error_t* errors[PARSER_MAX_ERROR_COUNT];
    unsigned int error_idx;
    scope_t scope;
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
parser_t create_parser();
program_t parse(parser_t *parser, token_t *tokens);
#endif
