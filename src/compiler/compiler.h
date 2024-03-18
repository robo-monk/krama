#ifndef H_COMPILER
#define H_COMPILER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"
#include "CCompiler.h"

string com_statement(Compiler *com, Statement *stmt);
void compile_program(BlockStatement *program, string filename);
#endif
