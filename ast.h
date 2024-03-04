#ifndef _AST_H
#define _AST_H

#include "tokeniser.h"

typedef struct AbstractNode AbstractNode;

struct AbstractNode
{
    AbstractNode *lhs;
    Token token;
    AbstractNode *rhs;
    AbstractNode *parent;
};

typedef struct
{
    AbstractNode *root;
} AST;

#include "parser.h"

AbstractNode *new_abstract_node(AbstractNode *lhs, Token token, AbstractNode *rhs);
AbstractNode *construct_abstract_node(Token *tokens, int token_index);
int evaluate_abstract_node(AbstractNode *node);



#endif
