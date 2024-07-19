#ifndef KRAMA_AST_H
#define KRAMA_AST_H
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
    EXPRESSION_TYPE_BIN,
    EXPRESSION_TYPE_LITERAL,
} expression_type_t;

typedef enum {
    ADD,
    SUB,
    MUL,
    DIV,
} bin_expression_op_t;

struct expression;

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
    struct expression *left;
    struct expression *right;
    bin_expression_op_t op;
} bin_expression_t;

typedef struct expression {
    union {
        bin_expression_t bin;
    } data;
} expression_t;

// STATEMENTS
typedef enum {
    STATEMENT_TYPE_LET,
    STATEMENT_TYPE_IDENTIFIER,
    STATEMENT_TYPE_BLOCK,
    STATEMENT_TYPE_DEFER,
} statement_type_t;

struct statement;

typedef struct {
    struct statement *statements;
    size_t statement_count;
    size_t statement_capacity;
} block_statement_t;

typedef struct {
   expression_t *expression;
} expression_statement_t;

typedef struct {
    struct statement *deferred_statement;
} defer_statement_t;

typedef struct {
    char* name;
} identifier_statement_t;

typedef struct {
    identifier_statement_t identifier;
} let_statement_t;

typedef struct statement {
    statement_type_t type;
    union {
        let_statement_t let;
        identifier_statement_t identifier;
        defer_statement_t defer;
        block_statement_t block;
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

#endif
