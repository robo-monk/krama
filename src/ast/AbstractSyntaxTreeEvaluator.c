#include "AbstractSyntaxTreeEvaluator.h"
#include "../hashmap/hashmap.h"
#include "AbstractSyntaxTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
  string name;
  LiteralType type;
  ReturnValue value;
} RuntimeVariable;

int runtime_variable_compare(const void *a, const void *b, void *udata) {
  const RuntimeVariable *ua = a;
  const RuntimeVariable *ub = b;
  return strcmp(ua->name, ub->name);
}

uint64_t runtime_variable_hash(const void *item, uint64_t seed0,
                               uint64_t seed1) {
  const RuntimeVariable *var = item;
  return hashmap_sip(var->name, strlen(var->name), seed0, seed1);
}

typedef struct {
  struct hashmap *map;
} Interpreter;

Interpreter new_interpreter() {
  const Interpreter i = {.map =
                             hashmap_new(sizeof(RuntimeVariable), 0, 0, 0,
                                         runtime_variable_hash,
                                         runtime_variable_compare, NULL, NULL)};
  return i;
}

ReturnValue write_variable(Interpreter *ipr, string var_name, LiteralType type,
                           ReturnValue value) {
  hashmap_set(ipr->map, &(RuntimeVariable){
                            .name = var_name, .type = type, .value = value});
  return value;
};

ReturnValue declare_variable(Interpreter *ipr, string var_name,
                             LiteralType type, ReturnValue value) {

  if (hashmap_get(ipr->map, &(RuntimeVariable){.name = var_name}) != NULL) {
    throw_runtime_error("redecleration of variable");
  }

  return write_variable(ipr, var_name, type, value);
}

ReturnValue read_variable(Interpreter *ipr, string var_name, LiteralType type) {
  const RuntimeVariable *v =
      hashmap_get(ipr->map, &(RuntimeVariable){.name = var_name});

  if (v == NULL) {
    throw_runtime_error("undeclared variable!");
  }

  return v->value;
};

ReturnValue evaluate_statement(Interpreter *ipr, Statement *stmt) {
  switch (stmt->type) {
  case LITERAL:
    return stmt->literal;
  case BIN_OP:
    return perform_bin_op(evaluate_statement(ipr, stmt->left),
                          evaluate_statement(ipr, stmt->right),
                          stmt->token.value.op_type);
  case VARIABLE_DECL:
    return write_variable(ipr, stmt->var_decl.name, stmt->var_decl.type,
                          evaluate_statement(ipr, stmt->right));
  case VARIABLE_READ:
    return read_variable(ipr, stmt->var_decl.name, stmt->var_decl.type);
  case VARIABLE_WRITE:
    return write_variable(ipr, stmt->var_decl.name, stmt->var_decl.type,
                          evaluate_statement(ipr, stmt->right));
  }

  throw_runtime_error("found unsupported token while evaluating");
}

ReturnValue exec_program(Program *program) {
  Interpreter ipr = new_interpreter();
  ReturnValue result = {};

  printf("evaluating program with len %d\n", program->len);
  for (int i = 0; i < program->len; i++) {
    printf("eval stmt #%d\n\n", i);
    result = evaluate_statement(&ipr, program->statements[i]);
  }

  return result;
}
