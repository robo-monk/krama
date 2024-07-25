#ifndef KRAMA_AST_H
#define KRAMA_AST_H

#include "tokeniser.h"
#include <stdlib.h>

// LITERAL TYPES

typedef enum {
    LITERAL_TYPE_I64,
    LITERAL_TYPE_F64,
    LITERAL_TYPE_CHARACTER,
    LITERAL_TYPE_STRING,
} literal_type_t;


// EXPRESSIONS
typedef enum {
    EXPRESSION_TYPE_PREFIX,
    EXPRESSION_TYPE_INFIX,
    EXPRESSION_TYPE_LITERAL,
    EXPRESSION_TYPE_IDENTIFIER,
    EXPRESSION_TYPE_FUNC_DECL,
    EXPRESSION_TYPE_BLOCK,
    EXPRESSION_TYPE_RETURN,
    EXPRESSION_TYPE_CONDITIONAL,
    EXPRESSION_TYPE_FOR,
    EXPRESSION_TYPE_CALL,
} expression_type_t;

typedef struct expression expression_t;


typedef struct {
    literal_type_t type;
    union {
        long i64;
        double f64;
        char character;
        char* string;
    } data;
} literal_expression_t;

typedef struct {
    expression_t *expression;
} return_expression_t;

typedef struct {
    expression_t *predicate;
    expression_t *success_branch;
    expression_t *fail_branch;
} conditional_expression_t;


typedef struct {
    char* name;
    expression_t *value;
} identifier_expression_t;

typedef struct {
    char* name;
    expression_t *value;
} func_decl_expression_t;


typedef struct {
    token_t operand;
    expression_t *right;
} prefix_expression_t;


typedef struct {
    token_t operand;
    expression_t *left;
    expression_t *right;
} infix_expression_t;

typedef struct {
    struct statement *statements;
    size_t statement_count;
    size_t statement_capacity;
} block_expression_t;

struct expression {
    expression_type_t type;
    union {
        infix_expression_t infix;
        prefix_expression_t prefix;
        literal_expression_t literal;
        identifier_expression_t identifier;
        block_expression_t block;
        func_decl_expression_t func_decl;
        return_expression_t return_exp;
        conditional_expression_t conditional;
    } data;
};

// STATEMENTS
typedef enum {
    STATEMENT_TYPE_LET,
    STATEMENT_TYPE_DEFER,
    STATEMENT_TYPE_EXPRESSION
} statement_type_t;

struct statement;
typedef struct {
   expression_t *expression;
} expression_statement_t;

typedef struct {
    struct statement *deferred_statement;
} defer_statement_t;

typedef struct {
    identifier_expression_t identifier;
} let_statement_t;

typedef struct statement {
    statement_type_t type;
    union {
        let_statement_t let;
        defer_statement_t defer;
        expression_t expression;
    } data;
} statement_t;

typedef struct {
    statement_t *statements;
    size_t statement_count;
    size_t statement_capacity;
} program_t;

program_t program_create(void);
void program_free(program_t *program);
int program_add_statement(program_t *program, statement_t statement);

block_expression_t block_expression_new(void);
int block_add_statement(block_expression_t *block, statement_t statement);

void statement_debug(statement_t *s, int ident);
#endif
