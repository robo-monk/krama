#ifndef _AST_H
#define _AST_H

#include "tokeniser.h"

typedef struct AbstractNode AbstractNode;

struct AbstractNode {
    AbstractNode *lhs;
    Token token; 
    AbstractNode *rhs;
    AbstractNode *parent;
}; 


typedef struct {
    AbstractNode *root;
} AST;

typedef struct
{
    int idx;
    Token *tokens;
} Parser;



AbstractNode* new_abstract_node(AbstractNode* lhs, Token token, AbstractNode* rhs);
AbstractNode *construct_abstract_node(Token *tokens, int token_index);

AbstractNode *parse_term(Parser *parser);
AbstractNode *parse_expression(Parser *parser);
AbstractNode *parse_factor(Parser *parser);

AbstractNode *parse(Parser *parser);

#endif
