#ifndef H_INFERER
#define H_INFERER
#include "../ast/AbstractSyntaxTree.h"
#include "../utils.h"
#include "CCompiler.h"

LiteralType infer_statement(Compiler *com, Statement *stmt);
#endif
