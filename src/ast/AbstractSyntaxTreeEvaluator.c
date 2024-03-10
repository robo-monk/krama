#include "AbstractSyntaxTreeEvaluator.h"
#include "../interpreter/Interpreter.h"
#include "AbstractSyntaxTree.h"

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
  }
  throw_runtime_error("unsupported operation for type");
  return 0;
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
  }
  throw_runtime_error("unsupported operation for type");
  return 0;
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

ReturnValue evaluate_statement(Interpreter *ipr, Statement *stmt) {
  switch (stmt->type) {
  case LITERAL:
    return stmt->literal;
  case BIN_OP:
    return perform_bin_op(evaluate_statement(ipr, stmt->left),
                          evaluate_statement(ipr, stmt->right),
                          stmt->token.value.op_type);
  case VARIABLE_DECL:
    return declare_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                            evaluate_statement(ipr, stmt->right));

  case VARIABLE_READ:
    return read_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type);
  case VARIABLE_WRITE:
    return write_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                          evaluate_statement(ipr, stmt->right));
  case IMPL_DECL:
    printf("declearing impl \n");
    return declare_implementation(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                                  stmt->right);
  case IMPL_CALL:
    printf("calling impl \n");
    return evaluate_statement(
        ipr, get_implementation_body(ipr, stmt->sym_decl.name));
    // const Statement **smts = malloc(1 * sizeof(Statement *));
    // return call_implementation(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
    //                            evaluate_statement(ipr, stmt->left));
  }

  throw_runtime_error("found unsupported token while evaluating");
}

ReturnValue exec_program(Program *program) {
  Interpreter ipr = new_interpreter();
  ReturnValue result = {};

  // printf("\n--|EXEC program with len %d\n", program->len);
  for (int i = 0; i < program->len; i++) {
    result = evaluate_statement(&ipr, program->statements[i]);
  }

  return result;
}
