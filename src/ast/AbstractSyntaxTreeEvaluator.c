#include "AbstractSyntaxTreeEvaluator.h"
#include "AbstractSyntaxTree.h"

int perform_i32_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op) {
  switch (op) {
  case TOKEN_OP_ADD:
    return lhs.i32_value + rhs.i32_value;
  case TOKEN_OP_MIN:
    return lhs.i32_value - rhs.i32_value;
  case MUL:
    return lhs.i32_value * rhs.i32_value;
  case DIV:
    return lhs.i32_value / rhs.i32_value;
  case TOKEN_OP_LT:
    if (lhs.i32_value < rhs.i32_value) {
      return 1;
    } else {
      return 0;
    }
  case TOKEN_OP_GT:
    if (lhs.i32_value > rhs.i32_value) {
      return 1;
    } else {
      return 0;
    }
  case TOKEN_OP_EQ:
    return lhs.i32_value == rhs.i32_value;
  case TOKEN_OP_GTE:
    return lhs.i32_value >= rhs.i32_value;
  case TOKEN_OP_LTE:
    return lhs.i32_value <= rhs.i32_value;
  case TOKEN_OP_BIN_AND:
  case TOKEN_OP_BIN_OR:
    throw_runtime_error("unsupported binary operation for type");
  }
  throw_runtime_error("unsupported operation for type");
  return 0;
}

double perform_f64_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op) {
  switch (op) {
  case TOKEN_OP_ADD:
    return lhs.f64_value + rhs.f64_value;
  case TOKEN_OP_MIN:
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
  // printf("\ncalling symbol: %s\n", sym_name);
  Statement *block_stmt = get_implementation_body(ipr, sym_name);
  // printf("\ngot block_stmt %d\n", block_stmt != NULL);
  Interpreter scope = new_interpreter();
  scope.upper_scope = ipr;

  if (block_stmt->block->args == NULL && block_stmt->block->arg_len == 0) {
    throw_runtime_error("wtf??");
  }

  Statement *args_statement = impl_call_stmt->right;
  if (args_statement->type != STMT_BLOCK) {
    throw_runtime_error("expected block statement that defines arguments");
  }

  for (int i = 0; i < block_stmt->block->arg_len; i++) {
    Argument *arg = block_stmt->block->args[i];
    ReturnValue arg_value =
        evaluate_statement(ipr, args_statement->block->statements[i]);

    declare_variable(&scope, arg->name, arg->type, arg_value);
  }

  ReturnValue rv = evaluate_statement(&scope, block_stmt);
  free_scope(&scope);
  return rv;
}

ReturnValue evaluate_conditional_statement(Interpreter *ipr,
                                           ConditionalStatement *conditional) {
  ReturnValue condition = evaluate_statement(ipr, conditional->condition);
  // printf("return value is %d", condition.i32_value);
  if (condition.i32_value == 1) {
    // printf("\ngo in conditional\n");
    return evaluate_block_statement(ipr, conditional->if_body);
  } else if (condition.i32_value == 0 && conditional->else_body != NULL) {
    return evaluate_block_statement(ipr, conditional->else_body);
    // printf("\n dont go in conditional\n");
  }
  // throw_runtime_error("undefined behaviour %d");
  printf("\n[WARNING] Statement evaluated to VOID\n");
  return (ReturnValue){};
}

ReturnValue evaluate_block_statement(Interpreter *ipr, BlockStatement *block) {
  for (int i = 0; i < block->len - 1; i++) {
    // printf("\n\n eval stmt %d\n", i);
    evaluate_statement(ipr, block->statements[i]);
  }
  return evaluate_statement(ipr, block->statements[block->len - 1]);
}

ReturnValue evaluate_statement(Interpreter *ipr, Statement *stmt) {
  switch (stmt->type) {
  case STMT_BLOCK:
    return evaluate_block_statement(ipr, stmt->block);
  case STMT_LITERAL:
    return stmt->literal;
  case STMT_BINARY_OP:
    return perform_bin_op(evaluate_statement(ipr, stmt->left),
                          evaluate_statement(ipr, stmt->right),
                          stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return declare_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                            evaluate_statement(ipr, stmt->right));

  case STMT_VARIABLE_READ:
    return read_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type);
  case STMT_VARIABLE_WRITE:
    return write_variable(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                          evaluate_statement(ipr, stmt->right));
  case STMT_DEF_DECL:
    return declare_implementation(ipr, stmt->sym_decl.name, stmt->sym_decl.type,
                                  stmt->right);
  case STMT_DEF_INVOKE:
    return call_symbol(ipr, stmt);
  case STMT_CONDITIONAL:
    // printf("\neval conditional:\n");
    // dbg_stmt(stmt);
    return evaluate_conditional_statement(ipr, stmt->conditional);
  }

  throw_runtime_error("found unsupported token while evaluating");
}

ReturnValue exec_program(BlockStatement *program) {
  Interpreter ipr = new_interpreter();
  ReturnValue result = {};

  for (int i = 0; i < program->len; i++) {
    result = evaluate_statement(&ipr, program->statements[i]);
  }

  return result;
}
