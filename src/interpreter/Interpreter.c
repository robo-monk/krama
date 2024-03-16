#include "Interpreter.h"
void throw_runtime_error(string msg) {
  printf("Runtime ERROR: %s", msg);
  exit(1);
}

RuntimeSymbol *new_runtime_sym(string name) {
  RuntimeSymbol *sym = malloc(sizeof(RuntimeSymbol));
  sym->name = name;
  return sym;
}

RuntimeSymbol *new_runtime_impl_symbol(string name, Statement *stmt) {
  RuntimeSymbol *sym = new_runtime_sym(name);
  RuntimeImplementation *impl = malloc(sizeof(RuntimeImplementation));
  impl->stmt = stmt;
  sym->impl = impl;
  return sym;
}

RuntimeSymbol *new_runtime_var_symbol(string name, LiteralType type,
                                      ReturnValue value) {

  RuntimeSymbol *sym = new_runtime_sym(name);
  sym->var = malloc(sizeof(RuntimeVariable));
  sym->var->type = type;
  sym->var->value = value;
  return sym;
}

int runtime_sym_compare(const void *a, const void *b, void *udata) {
  const RuntimeSymbol *ua = a;
  const RuntimeSymbol *ub = b;

  // printf("\ncompare (%s) & (%s)\n", ua->name, ub->name);
  return strcmp(ua->name, ub->name);
}

uint64_t runtime_sym_hash(const void *item, uint64_t seed0, uint64_t seed1) {
  const RuntimeSymbol *var = item;
  // printf("\nhash (%s)", var->name);
  return hashmap_sip(var->name, strlen(var->name), seed0, seed1);
}

Interpreter new_interpreter() {
  const Interpreter i = {.map = hashmap_new(sizeof(RuntimeSymbol), 0, 0, 0,
                                            runtime_sym_hash,
                                            runtime_sym_compare, NULL, NULL),
                         .upper_scope = NULL};
  return i;
}

void free_scope(Interpreter *scope) { hashmap_free(scope->map); }

RuntimeSymbol const *get_symbol(Interpreter *ipr, string sym_name) {
  // return hashmap_get(ipr->map, &(RuntimeSymbol){.name = sym_name});
  return hashmap_get(ipr->map, &(RuntimeSymbol){.name = sym_name});
}

RuntimeVariable *get_var_symbol(Interpreter *ipr, string sym_name) {
  const RuntimeSymbol *sym = get_symbol(ipr, sym_name);
  if (sym == NULL)
    return NULL;

  return sym->var;
}

RuntimeImplementation *get_impl_symbol(Interpreter *ipr, string sym_name) {

  const RuntimeSymbol *sym = get_symbol(ipr, sym_name);
  if (sym == NULL) {
    return NULL;
  }

  return sym->impl;
}

ReturnValue write_variable(Interpreter *ipr, string var_name, LiteralType type,
                           ReturnValue value) {

  hashmap_set(ipr->map, new_runtime_var_symbol(var_name, type, value));
  return value;
};

ReturnValue declare_variable(Interpreter *ipr, string var_name,
                             LiteralType type, ReturnValue value) {

  // printf("   DECLARE %s = %d\n", var_name, value.i32_value);
  if (get_var_symbol(ipr, var_name) != NULL) {
    throw_runtime_error("redecleration of variable");
  }

  return write_variable(ipr, var_name, type, value);
}

const RuntimeSymbol *read_symbol(Interpreter *ipr, string sym_name) {
  const RuntimeSymbol *sym = get_symbol(ipr, sym_name);
  // bubble up scope
  if (sym == NULL) {
    if (ipr->upper_scope != NULL) {
      // printf("\nbubble up scope\n");
      return read_symbol(ipr->upper_scope, sym_name);
    }

    printf("usage of undeclared symbol '%s'!", sym_name);
    throw_runtime_error("\n");
  }

  return sym;
};

ReturnValue read_variable(Interpreter *ipr, string var_name, LiteralType type) {
  const RuntimeSymbol *sym = read_symbol(ipr, var_name);
  if (sym->var == NULL) {
    throw_runtime_error("undeclared variable!");
  }
  return sym->var->value;
};

ReturnValue declare_implementation(Interpreter *ipr, string impl_name,
                                   LiteralType type, Statement *stmt) {

  if (get_impl_symbol(ipr, impl_name) != NULL) {
    throw_runtime_error("impl already declared");
  }

  const RuntimeSymbol *impl = new_runtime_impl_symbol(impl_name, stmt);
  hashmap_set(ipr->map, impl);
  return (ReturnValue){.type = NULL};
}

Statement *get_implementation_body(Interpreter *ipr, string impl_name) {
  const RuntimeSymbol *sym = read_symbol(ipr, impl_name);
  if (sym->impl == NULL) {
    throw_runtime_error("undeclared implementation!");
  }
  return sym->impl->stmt;
}

// ReturnValue call_implementation(Interpreter *ipr, string var_name,
//                                 LiteralType type, ReturnValue _) {

//   const RuntimeSymbol *sym = hashmap_get(ipr->map,
//   new_runtime_sym(var_name));

//   Argument *arg = sym->impl->stmt->block->arg;
//   declare_variable(ipr, arg->name, arg->type, evaluate_statement(ipr,
//   sym->impl->stmt->right));

//   if (sym == NULL) {
//     throw_runtime_error("undeclared implementation!");
//   }

//   // return (sym->impl->stmts)[0]
//   // return sym->value;
// };
