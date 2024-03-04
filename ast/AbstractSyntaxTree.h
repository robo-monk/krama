#ifndef H_AST
#define H_AST

#include "../tokeniser.h"
#include "../utils.h"

typedef enum { BIN_OP, LITERAL } StatementType;

typedef enum {
  LiteralType_i32,
  LiteralType_f64,
  LiteralType_cha,
} LiteralType;

typedef struct {
  LiteralType type;
  union {
    int i32_value;
    double f64_value;
    char cha_value;
  };
} LiteralStatement;

typedef struct {
  OpType op;
} BinExpressionStatement;

typedef struct Statement Statement;

struct Statement {
  union {
    LiteralStatement literal;
    BinExpressionStatement bin_op;
  };
  StatementType type;
  Statement *left;
  Statement *right;

  Token token;
};

typedef struct {
  Statement **statements;
  unsigned int idx;
  unsigned int len;
  unsigned int max_len;
} Program;

Program *new_program();
void push_stmt(Statement *stmt, Program *program);

Statement *new_stmt(StatementType type, Statement *left, Statement *right,
                    Token token);
Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right,
                             Token token);
Statement *new_i32_literal_stmt(int value, Token token);
Statement *new_f64_literal_stmt(double value, Token token);

#endif
