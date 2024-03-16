#ifndef H_COMPILER
#define H_COMPILER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"

typedef struct {
  LiteralType type;
} VariableSymbol;

typedef struct {
  Statement *stmt;
  LiteralType type;
} DefSymbol;

typedef struct {
  string name;
  union {
    DefSymbol *def;
    VariableSymbol *var;
  };
} Symbol;

typedef struct {
  Vec headers;
  Vec implementations;
  struct hashmap *sym_map;
} CCompiler;

string com_statement(CCompiler *com, Statement *stmt);
void compile_program(BlockStatement *program, string filename);
#endif
