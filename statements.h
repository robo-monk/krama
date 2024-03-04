#ifndef _STATEMENTS_H

#define _STATEMENTS_H
#include "tokeniser.h"
#include "utils.h"

typedef enum {
    BinOp,
    Literal
} StatementType;

typedef enum {
    LiteralType_i32,
    LiteralType_f64
} LiteralType;

typedef struct {
    LiteralType type;
    union {
        int i32_value;
        double f64_value;
    };
} LiteralStatement;

typedef struct {
    OpType op;
} BinExpressionStatement;

typedef struct Statement Statement;

typedef struct {
    union {
        LiteralStatement literal;
        BinExpressionStatement bin_op;
    };
    StatementType type;
    Statement *left;
    Statement *right;

    string raw_value;
    unsigned int start;
    unsigned int end;
} Statement;


Statement new_stmt(StatementType type, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value);
Statement new_bin_expr_stmt(OpType op, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value);
Statement new_i32_literal_stmt(int value, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value);
Statement new_f64_literal_stmt(double value, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value);
#endif
