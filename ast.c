#include "stdlib.h"
#include "utils.h"
#include "ast.h"

AbstractNode new_abstract_node(AbstractNode* lhs, OpType op, AbstractNode* rhs) {
    AbstractNode node = {
        .op = op,
        .lhs = lhs,
        .rhs = rhs
    };

    return node;
}


int perform_op(int lhs, int rhs, OpType op)
{
    switch (op)
    {
    case ADD:
        return lhs + rhs;
    case MIN:
        return lhs - rhs;
    case MUL:
        return lhs * rhs;
    case DIV:
        return lhs + rhs;
    default:
        printf("urecognised OpType");
    }
}

int evaluate_abstract_node(AbstractNode* node) {
    if (node->token.type == NUMBER) {
        return node->token.value.n;
    }

    if (node->token.type == OP) {
        return perform_op(evaluate_abstract_node(node->lhs), evaluate_abstract_node(node->rhs), node->token.value.op_type);
    }

    printf("found unexpected token while evaluate ast");
    exit(1);
}

int evaluate_ast(AST *ast) {
    return evaluate_abstract_node(ast->root);
}
