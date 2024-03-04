#include "stdio.h"
#include "tokeniser.h"
#include "stdlib.h"
#include "parser.h"

Token look_ahead(Parser *parser) {

    return parser->tokens[parser->idx + 1];
}

Token peek(Parser *parser)
{
    return parser->tokens[parser->idx];
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

    if (current_token.type == NUMBER)
    {
        return new_abstract_node(NULL, current_token, NULL);
    }
    else if (current_token.type == OPEN_PAR)
    {
        AbstractNode *node = parse_expression(parser);
        expect(parser, CLOSE_PAR, "Expected closing paren");
        eat(parser);
        return node;
    }

    throw_parser_error(parser, "Unexpected token");
    return NULL;
}

AbstractNode *parse_term(Parser *parser)
{
    AbstractNode *current_node = parse_factor(parser);
    Token current_token = peek(parser);

    while (current_token.type == OP &&
           (current_token.value.op_type == MUL || current_token.value.op_type == DIV))
    {
        
        eat(parser);
        AbstractNode *lhs = current_node;
        AbstractNode *rhs = parse_factor(parser);
        current_node = new_abstract_node(lhs, current_token, rhs);
        current_token = peek(parser);
    }

    return current_node;
}

AbstractNode *parse_expression(Parser *parser)
{
    AbstractNode *current_node = parse_term(parser);
    Token current_token = peek(parser);

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
    AbstractNode *node = parse_expression(parser);

    if (peek(parser).type != PROGRAM_END) {
        throw_parser_error(parser, "premature exit.");
    }

    return node;
}
