#ifndef H_COMPILER
#define H_COMPILER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"

typedef struct CCompiler CCompiler;

typedef struct Scope Scope;
struct Scope {
  struct hashmap *sym_map;
  Scope *upper;
};

typedef struct {
  LiteralType type;
} VariableSymbol;

typedef struct {
  Statement *body;
  LiteralType type;
} DefSymbol;

typedef struct {
  string name;
  union {
    DefSymbol *def;
    VariableSymbol *var;
  };
} Symbol;

struct CCompiler {
  StrVec headers;
  StrVec declerations;
  StrVec implementations;

  // TODO: this should be a Scope struct
  // Scope *scope;

  struct hashmap *sym_map;
  CCompiler *upper;
};

string com_statement(CCompiler *com, Statement *stmt);
void compile_program(BlockStatement *program, string filename);
#endif
