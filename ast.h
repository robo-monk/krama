#ifndef _AST_H
#define _AST_H

#include "tokeniser.h"

typedef struct {
    AbstractNode *lhs;
    Token token; 
    AbstractNode *rhs;
} AbstractNode;

typedef struct {
    AbstractNode *root;
} AST;

#endif
