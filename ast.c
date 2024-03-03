#include <stdio.h>
#include "stdlib.h"
#include "utils.h"
#include "ast.h"

AbstractNode *new_abstract_node(AbstractNode *lhs, Token token, AbstractNode *rhs)
{
    AbstractNode *node = (AbstractNode *)malloc(sizeof(AbstractNode));
    node->lhs = lhs;
    node->rhs = rhs;
    node->token = token;
    return node;
}

// int perform_op(int lhs, int rhs, OpType op)
// {
//     switch (op)
//     {
//     case ADD:
//         return lhs + rhs;
//     case MIN:
//         return lhs - rhs;
//     case MUL:
//         return lhs * rhs;
//     case DIV:
//         return lhs + rhs;
//     default:
//         printf("urecognised OpType");
//     }
// }

int evaluate_abstract_node(AbstractNode *node)
{
    if (node->token.type == NUMBER)
    {
        return node->token.value.n;
    }

    if (node->token.type == OP)
    {
        // return perform_op(evaluate_abstract_node(node->lhs), evaluate_abstract_node(node->rhs), node->token.value.op_type);
    }

    printf("found unexpected token while evaluate ast");
    exit(1);
}

Token peek(Parser *parser)
{
    return parser->tokens[(parser->idx)];
}

Token eat(Parser *parser)
{
    return parser->tokens[parser->idx++];
}

void throw_parser_error(Parser *parser, string error_msg)
{
    printf("[1:%d] ", parser->idx);
    printf("Parser Error: %s \n", error_msg);
    exit(1);
}

void expect(Parser *parser, TokenType token_type, string error_msg)
{
    if (peek(parser).type != token_type)
    {
        throw_parser_error(parser, error_msg);
    }
}

Parser new_parser(Token *tokens)
{
    Parser p = {
        .idx = 0,
        .tokens = tokens};

    return p;
}

AbstractNode *parse_factor(Parser *parser)
{
    Token current_token = eat(parser);
    printf("[parse fact] next token is: ");
    dbg_token(current_token);
    printf("\n");

    // LEAF
    if (current_token.type == NUMBER)
    {
        return new_abstract_node(NULL, current_token, NULL);
    }
    else if (current_token.type == OPEN_PAR)
    {
        AbstractNode *node = parse_expression(parser);
        expect(parser, CLOSE_PAR, "Expected closing paren");
        return node;
    }

    throw_parser_error(parser, "Unexpected token");
}

AbstractNode *parse_term(Parser *parser)
{
    AbstractNode *current_node = parse_factor(parser);
    Token current_token = peek(parser);

    printf("[parse term] next token is: ");
    dbg_token(current_token);
    printf("\n");


    while (current_token.type == OP &&
           (current_token.value.op_type == MUL || current_token.value.op_type == DIV))
    {
        eat(parser);
        AbstractNode *lhs = current_node;
        AbstractNode *rhs = parse_expression(parser);
        current_node = new_abstract_node(lhs, current_token, rhs);
        current_token = peek(parser);
    }

    return current_node;
}

AbstractNode *parse_expression(Parser *parser)
{
    AbstractNode *current_node = parse_term(parser);
    Token current_token = peek(parser);
    printf("[parse expr] next token is: ");
    dbg_token(current_token);
    printf("\n");

    while (current_token.type == OP &&
           (current_token.value.op_type == ADD || current_token.value.op_type == MIN))
    {
        eat(parser);
        AbstractNode *lhs = current_node;
        AbstractNode *rhs = parse_term(parser);
        current_node = new_abstract_node(lhs, current_token, rhs);
        current_token = peek(parser);
    }

    return current_node;
}

AbstractNode *parse(Parser *parser)
{
    return parse_expression(parser);
}

AbstractNode *construct_abstract_node(Token *tokens, int token_index)
{
    Parser parser = new_parser(tokens);
    return parse(&parser);
}

int evaluate_ast(AST *ast)
{
    return evaluate_abstract_node(ast->root);
}
