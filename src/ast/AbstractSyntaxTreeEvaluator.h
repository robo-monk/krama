#ifndef H_AST_EVALUATOR
#define H_AST_EVALUATOR

#include "../interpreter/Interpreter.h"
#include "AbstractSyntaxTree.h"

// for now every  statement can be redu
typedef LiteralStatement ReturnValue;
ReturnValue exec_program(BlockStatement *program);
ReturnValue evaluate_statement(Interpreter *ipr, Statement *stmt);
#endif
