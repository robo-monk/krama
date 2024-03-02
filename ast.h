#ifndef _AST_H
#define _AST_H

#include "tokeniser.h"

typedef struct AbstractNode AbstractNode;

struct AbstractNode {
    AbstractNode *lhs;
    Token token; 
    AbstractNode *rhs;
}; 


typedef struct {
    AbstractNode *root;
} AST;

AbstractNode* new_abstract_node(AbstractNode* lhs, Token token, AbstractNode* rhs);

#endif
