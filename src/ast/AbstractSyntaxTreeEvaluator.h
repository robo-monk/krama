#ifndef H_AST_EVALUATOR
#define H_AST_EVALUATOR

#include "AbstractSyntaxTree.h"

// for now every  statement can be redu
typedef LiteralStatement ReturnValue;
ReturnValue exec_program(Program *program);

#endif
