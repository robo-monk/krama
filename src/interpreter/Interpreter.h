#ifndef H_INTERPRETER
#define H_INTERPRETER

#include "../ast/AbstractSyntaxTree.h"
// #include "../ast/AbstractSyntaxTreeEvaluator.h"
#include "../hashmap/hashmap.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef LiteralStatement ReturnValue;
typedef struct Interpreter Interpreter;

typedef struct {
  LiteralType type;
  ReturnValue value;
} RuntimeVariable;

typedef struct {
  // TODO this should hold a BlockStatement
  Statement *stmt;
} RuntimeImplementation;

typedef struct {
  string name;
  union {
    RuntimeImplementation *impl;
    RuntimeVariable *var;
  };
} RuntimeSymbol;

struct Interpreter {
  struct hashmap *map;
  Interpreter *upper_scope;
};

void throw_runtime_error(string msg);
Interpreter new_interpreter();
void free_scope(Interpreter *scope);

ReturnValue declare_variable(Interpreter *ipr, string var_name,
                             LiteralType type, ReturnValue value);

ReturnValue write_variable(Interpreter *ipr, string var_name, LiteralType type,
                           ReturnValue value);

ReturnValue read_variable(Interpreter *ipr, string var_name, LiteralType type);

ReturnValue declare_implementation(Interpreter *ipr, string impl_name,
                                   LiteralType type, Statement *stmt);

ReturnValue call_implementation(Interpreter *ipr, string var_name,
                                LiteralType type, ReturnValue arg);

Statement *get_implementation_body(Interpreter *ipr, string impl_name);
#endif
