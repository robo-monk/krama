#include "AbstractSyntaxTreeEvaluator.h"
#include <stdio.h>
#include <stdlib.h>

void throw_runtime_error(string msg) {
  printf("%s", msg);
  exit(1);
}

int perform_i32_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op) {
  switch (op) {
  case ADD:
    return lhs.i32_value + rhs.i32_value;
  case MIN:
    return lhs.i32_value - rhs.i32_value;
  case MUL:
    return lhs.i32_value * rhs.i32_value;
  case DIV:
    return lhs.i32_value / rhs.i32_value;
  default:
    throw_runtime_error("unsupported operation for type");
  }
}

double perform_f64_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op) {
  switch (op) {
  case ADD:
    return lhs.f64_value + rhs.f64_value;
  case MIN:
    return lhs.f64_value - rhs.f64_value;
  case MUL:
    return lhs.f64_value * rhs.f64_value;
  case DIV:
    return lhs.f64_value / rhs.f64_value;
  default:
    throw_runtime_error("unsupported operation for type");
  }
}

ReturnValue perform_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op) {
  if (lhs.type != rhs.type)
    throw_runtime_error("Unmatched types!");

  ReturnValue v = {.type = lhs.type};

  switch (lhs.type) {
  case LiteralType_i32:
    v.i32_value = perform_i32_bin_op(lhs, rhs, op);
    break;
  case LiteralType_f64:
    v.f64_value = perform_f64_bin_op(lhs, rhs, op);
    break;
  default:
    throw_runtime_error("unsupported operation for type");
  }

  return v;
}

ReturnValue evaluate_statement(Statement *stmt) {
  switch (stmt->type) {
  case LITERAL:
    return stmt->literal;
  case BIN_OP:
    return perform_bin_op(evaluate_statement(stmt->left),
                          evaluate_statement(stmt->right),
                          stmt->token.value.op_type);
  }

  throw_runtime_error("unsupported token");
}

ReturnValue exec_program(Program *program) {
  ReturnValue result = {};

  for (int i = 0; i < program->len; i++) {
    result = evaluate_statement(program->statements[i]);
  }

  return result;
}
