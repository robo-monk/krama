#ifndef H_INFERER
#define H_INFERER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"
#include "CCompiler.h"

typedef struct {
  Compiler *com;
  Statement *current_stmt;
} Inferer;

Inferer Inferer_new(Compiler *com);
LiteralType infer_statement(Inferer *inf, Statement *stmt);
LiteralType infer_var_read(Inferer *inf, Statement *stmt);
#endif
