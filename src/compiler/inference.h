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
BranchLiteral infer_statement(Inferer *inf, Statement *stmt);
void dbg_branch_literal(BranchLiteral *b);
LiteralType converge_branch_literal(Inferer *inf, BranchLiteral *b);
BranchLiteral new_for_literal(LiteralType lit);

#endif
