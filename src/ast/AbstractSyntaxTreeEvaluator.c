#include "AbstractSyntaxTreeEvaluator.h"
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

// ReturnValue call_implementation()
ReturnValue call_symbol(Interpreter *ipr, Statement *impl_call_stmt) {
  string sym_name = impl_call_stmt->sym_decl.name;
  printf("\ncalling symbol: %s\n", sym_name);
  Statement *block_stmt = get_implementation_body(ipr, sym_name);
  printf("\ngot block_stmt %d\n", block_stmt != NULL);
  Interpreter scope = new_interpreter();
  scope.upper_scope = ipr;
  // declare vars
  // TODO: implement scope
  Argument *arg = block_stmt->block->arg;
  if (arg != NULL) {
    // printf("\ndeclaring argument with name: '%s'", arg->name);
    declare_variable(&scope, arg->name, arg->type,
                     evaluate_statement(ipr, impl_call_stmt->right));
  }
  ReturnValue rv = evaluate_statement(&scope, block_stmt);
  free_scope(&scope);
  return rv;
}

ReturnValue evaluate_statement(Interpreter *ipr, Statement *stmt) {
  switch (stmt->type) {
  case BLOCK:
    // TODO implement scope

    for (int i = 0; i < stmt->block->len - 1; i++) {
      // evaluate_statement(ipr, stmt);
      evaluate_statement(ipr, stmt->block->statements[i]);
    }
    return evaluate_statement(ipr,
                              stmt->block->statements[stmt->block->len - 1]);

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
    return declare_implementation(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                                  stmt->right);
  case IMPL_CALL:
    // get_implementation_body(ipr, stmt->sym_decl.name);
    // return evaluate_statement(ipr, );
    // const Statement **smts = malloc(1 * sizeof(Statement *));
    return call_symbol(ipr, stmt);
  }

  throw_runtime_error("found unsupported token while evaluating");
}

ReturnValue exec_program(BlockStatement *program) {
  Interpreter ipr = new_interpreter();
  ReturnValue result = {};

  // printf("\n--|EXEC program with len %d\n", program->len);
  for (int i = 0; i < program->len; i++) {
    result = evaluate_statement(&ipr, program->statements[i]);
  }

  return result;
}
