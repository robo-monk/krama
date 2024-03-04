#include "./AbstractSyntaxTree.h"
#include "stdio.h"
#include "stdlib.h"

Statement *new_stmt(StatementType type, Statement *left, Statement *right,
                    Token token) {
  Statement *s = malloc(sizeof(Statement));
  s->type = type;
  s->left = left;
  s->right = right;
  s->token = token;
  return s;
}

Statement *new_var_decl_stmt(LiteralType type, string name, Statement *left,
                             Statement *right, Token token) {
  Statement *s = new_stmt(VARIABLE_DECL, left, right, token);
  s->var_decl.name = name;
  s->var_decl.type = type;
  return s;
}

Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right,
                             Token token) {
  Statement *s = new_stmt(BIN_OP, left, right, token);
  s->bin_op.op = op;
  return s;
}

Statement *new_i32_literal_stmt(int value, Token token) {
  Statement *s = new_stmt(LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_i32;
  s->literal.i32_value = value;
  return s;
}

Statement *new_f64_literal_stmt(double value, Token token) {
  Statement *s = new_stmt(LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_f64;
  s->literal.f64_value = value;
  return s;
}

void _alloc_statement_array(Program *program) {
  if (program->len < program->max_len) {
    return;
  }

  program->max_len = program->max_len * 2;
  Statement **new_mem =
      realloc(program->statements, sizeof(Statement) * program->max_len);

  if (new_mem == NULL) {
    return;
  }
  program->statements = new_mem;
}

#define DEFAULT_PROGRAM_LEN 100;
Program *new_program() {
  Program *program = malloc(sizeof(Program));
  program->idx = 0;
  program->len = 0;
  program->max_len = DEFAULT_PROGRAM_LEN;
  program->statements = malloc(sizeof(Statement) * program->max_len);
  return program;
}

void push_stmt(Statement *stmt, Program *program) {
  program->len += 1;
  // optionally alloc more memory
  _alloc_statement_array(program);
  program->statements[program->len - 1] = stmt;
}
