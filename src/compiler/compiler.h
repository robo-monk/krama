#ifndef H_COMPILER
#define H_COMPILER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"

typedef struct {
  Vec headers;
  Vec implementations;
} CCompiler;

string com_statement(CCompiler *com, Statement *stmt);
void compile_program(BlockStatement *program, string filename);
#endif
