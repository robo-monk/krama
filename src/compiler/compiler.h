#ifndef H_COMPILER
#define H_COMPILER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"

typedef struct {
  Vec headers;
  Vec implementations;
} CCompiler;

void compile_program(BlockStatement *program, string filename);
#endif
