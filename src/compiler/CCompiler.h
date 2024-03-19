#ifndef H_CCOMPILER
#define H_CCOMPILER

#include "../frontend/LiteralType.h"
#include "../hashmap/hashmap.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct Compiler Compiler;

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

struct Compiler {
  StrVec headers;
  StrVec declerations;
  StrVec implementations;

  // TODO: this should be a Scope struct
  // Scope *scope;
  struct hashmap *sym_map;
  Compiler *upper;

  Statement *current_stmt;
};

// void Compiler_throw(const char *format, ...);
void Compiler_throw(Compiler *com, const char *fmt, ...);

Symbol *new_sym(string name);
Symbol *new_def_symbol(string name, Statement *body, LiteralType return_type);
Symbol *new_var_symbol(string name, LiteralType type);

Symbol const *Compiler_symbol_get(Compiler *com, string sym_name);

VariableSymbol *Compiler_varsym_get(Compiler *com, string var_name);

DefSymbol *Compiler_defsym_get(Compiler *com, string def_name);

void Compiler_defsym_declare(Compiler *com, string def_name, Statement *body,
                             LiteralType return_type);

void Compiler_varsym_declare(Compiler *com, string var_name, LiteralType type);

Compiler Compiler_new();
void Compiler_free(Compiler *com);

void Compiler_impl_push(Compiler *com, string impl_str);
void Compiler_decl_push(Compiler *com, string decl_str);
void Compiler_header_push(Compiler *com, string head_str);
void Compiler_write_to_file(Compiler *com, string filename);

#endif
