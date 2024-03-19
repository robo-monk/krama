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
BranchLiteral *infer_statement(Inferer *inf, Statement *stmt);
void dbg_branch_literal(BranchLiteral *b);

BranchLiteral *BranchLiteral_new(LiteralType *lit);
LiteralType BranchLiteral_converge(Inferer *inf, BranchLiteral *b);

#endif
