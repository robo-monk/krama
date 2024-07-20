#include "parser.h"
#include "ast.h"
#include "tokeniser.h"
#include <string.h>

token_t parser_peek(parser_t *parser) {
    return parser->tokens[parser->index+1];
}

token_t parser_current(parser_t *parser) {
    return parser->tokens[parser->index];
}

void parser_debug(parser_t *parser, char* msg) {
    printf("\n[DEBUG] Parser: %s ", msg);;
    token_debug(parser_current(parser));
}

token_t parser_eat(parser_t *parser) {
    return parser->tokens[parser->index++];
}

token_t parser_expect(parser_t *parser, token_type_t token_type) {
    token_t current = parser_current(parser);
    if (current.type != token_type) {
        printf("\nExpected ");
        token_debug((token_t) { .type = token_type });
        printf(" but got: ");
        token_debug(current);
        printf("\n\n");
        exit(1);
    }
    return current;
}

token_t parser_eat_and_expect(parser_t *parser, token_type_t token_type) {
    token_t current = parser_eat(parser);
    if (current.type != token_type) {
        printf("\nExpected ");
        token_debug((token_t) { .type = token_type });
        printf(" but got: ");
        token_debug(current);
        printf("\n\n");
        exit(1);
    }
    return current;
}

literal_expression_t parser_parse_literal(parser_t *parser) {
    token_t current = parser_eat_and_expect(parser, TOKEN_LITERAL);
    return (literal_expression_t) {
        .type = LITERAL_TYPE_I64,
        .data = {
            .i64 = atoi(current.value.raw_str)
        }
    };
}


void add_tabs(int count) {
    for (int i = 0; i < count; i++) {
        printf("\t");
    }
}

void debug_expression(expression_t *expression, int ident) {
    if (expression == NULL) {
        printf("NULL");
        return;
    }
    // printf("%s", ident);
    switch (expression->type) {
    case EXPRESSION_TYPE_PREFIX:
        printf("expr PREFIX (");
        token_debug(expression->data.prefix.operand);
        printf(")\n");
        add_tabs(ident);
        printf("\tR: ");
        debug_expression(expression->data.prefix.right, ident+1);
        break;
    case EXPRESSION_TYPE_INFIX:
        printf("expr INFIX (");
        token_debug(expression->data.infix.operand);
        printf(")\n");
        add_tabs(ident);
        printf("\tL: ");
        debug_expression(expression->data.infix.left, ident+1);
        printf("\n");
        add_tabs(ident);
        printf("\tR: ");
        debug_expression(expression->data.infix.right, ident+1);
        break;
    case EXPRESSION_TYPE_LITERAL:
        // add_tabs(ident);
        printf("expr LITERAL (%ld)", expression->data.literal.data.i64);
        break;
    case EXPRESSION_TYPE_IDENTIFIER:
        // add_tabs(ident);
        printf("expr IDENTIFIER (%s)", expression->data.identifier.name);;
        break;
    }
}

void statement_debug(statement_t *s, int ident) {
    switch (s->type) {
    case STATEMENT_TYPE_LET: {
        add_tabs(ident);
        printf("LET `%s`", s->data.let.identifier.name);
        printf("\n");
        // printf("\n│\n");
        add_tabs(ident);
        printf("└─  ");
        return debug_expression(s->data.let.identifier.value, ident+1);
    }
    case STATEMENT_TYPE_EXPRESSION: {
        return debug_expression(&s->data.expression, ident);
    }
    case STATEMENT_TYPE_BLOCK:
    case STATEMENT_TYPE_DEFER:
        printf("\nnot implemented\n");
        break;
    }
}


precedence_t get_precedence(token_type_t token_type) {
    switch (token_type) {
        // case TOKEN_L_BRACKET:
        // case TOKEN_R_BRACKET:
        // case TOKEN_L_PAREN:
        // case TOKEN_R_PAREN:
        case TOKEN_COLON:
        case TOKEN_BANG:
            return PRECEDENCE_PREFIX;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return PRECEDENCE_SUM;
        case TOKEN_DIV:
        case TOKEN_MULT:
            return PRECEDENCE_PROD;
        default:
            return PRECEDENCE_LOWEST;
    }
}

expression_t* parser_parse_expression(parser_t *parser, precedence_t precedence);

expression_t* parser_parse_prefix_expression(parser_t *parser) {
    expression_t *expr = malloc(sizeof(expression_t));
    // parse prefix
    switch (parser_current(parser).type) {
        case TOKEN_LITERAL: {
            expr->type = EXPRESSION_TYPE_LITERAL;
            expr->data.literal = parser_parse_literal(parser);
            break;
        }

        case TOKEN_PLUS:
        case TOKEN_BANG:
        case TOKEN_MINUS: {
            expr->type = EXPRESSION_TYPE_PREFIX;
            expr->data.prefix = (prefix_expression_t) {
                .operand = parser_eat(parser),
                .right = parser_parse_expression(parser, PRECEDENCE_PREFIX)
            };
            break;
        }
        case TOKEN_IDENTIFIER: {
            expr->type = EXPRESSION_TYPE_IDENTIFIER;
            expr->data.identifier = (identifier_expression_t) {
                .name = strdup(parser_eat(parser).value.raw_str)
            };
            break;
        }
        default:
            free(expr);
            return NULL;
            // printf("\nInvalid token!\n");
            // token_debug(parser_peek(parser));
            // exit(1);
    }
    return expr;
}

expression_t* parser_parse_infix_expression(parser_t *parser, expression_t *left) {
    expression_t *expr = malloc(sizeof(expression_t));
    // parse prefix
    switch (parser_current(parser).type) {
        // case TOKEN_LITERAL: {
        //     expr->type = EXPRESSION_TYPE_LITERAL;
        //     expr->data.literal = parser_parse_literal(parser);
        //     break;
        // }
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULT: {
            parser_debug(parser, "INFIX as");
            expr->type = EXPRESSION_TYPE_INFIX;
            token_t operand = parser_eat(parser);
            precedence_t precedence = get_precedence(operand.type);
            expr->data.infix = (infix_expression_t) {
                .operand = operand,
                .right = parser_parse_expression(parser, precedence),
                .left = left
            };
            break;
        }
        default:
            free(expr);
            return NULL;
            // return expr;;
            // printf("\nInvalid token!\n");
            // token_debug(parser_current(parser));
            // printf("\n");
            // exit(1);
    }
    printf("\n DEBUG INFIX \n");
    debug_expression(expr, 0);
    printf("\n -- DEBUG INFIX -- \n");
    return expr;
}

expression_t* parser_parse_expression(parser_t *parser, precedence_t precedence) {
    expression_t *expr = parser_parse_prefix_expression(parser);
    parser_debug(parser, "after prefix parsing");
    if (expr == NULL) return NULL;
    token_t next;
    while (
        next = parser_current(parser),
        next.type != TOKEN_SEMICOLON
        && next.type != TOKEN_NEW_LINE
        && next.type != TOKEN_EOF
        && precedence < get_precedence(next.type)) {
        expression_t *infix_expr = parser_parse_infix_expression(parser, expr);
        if (infix_expr == NULL) return expr;
        expr = infix_expr;
    }
    return expr;
}

void parser_parse_statement(parser_t *parser) {
}

void parser_parse_block(parser_t *parser) {
}

void parser_parse(parser_t *parser) {
    token_t current;
    while (current = parser_current(parser), current.type != TOKEN_EOF) {

        switch (current.type) {
            case TOKEN_SEMICOLON:
            case TOKEN_NEW_LINE:
                break;
            case TOKEN_LET: {
                token_t let = parser_eat(parser);
                token_t identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);
                token_t eq = parser_eat_and_expect(parser, TOKEN_EQ);
                program_add_statement(&parser->program, (statement_t) {
                    .type = STATEMENT_TYPE_LET,
                    .data.let = {
                        .identifier = {
                            .name = strdup(identifier.value.raw_str),
                            .value = parser_parse_expression(parser, PRECEDENCE_LOWEST)
                        }
                    }
                });
                break;
            }
            default:
                program_add_statement(&parser->program, (statement_t) {
                    .type = STATEMENT_TYPE_EXPRESSION,
                    .data.expression = *parser_parse_expression(parser, PRECEDENCE_LOWEST)
                });
        }
        parser_eat(parser);
    }
}

program_t parse(token_t *tokens) {
    parser_t parser = (parser_t) {
        .index = 0,
        .tokens = tokens,
        .program = program_create()
    };

    parser_parse(&parser);

    for (int i = 0; i < parser.program.statement_count; i++) {
        printf("\nSTATEMENT #%d\n", i);
        statement_debug(&parser.program.statements[i], 0);
        printf("\n");
    }
    return parser.program;
}
