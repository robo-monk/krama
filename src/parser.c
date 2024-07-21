#include "parser.h"
#include "ast.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <string.h>

void parser_error_print(parser_error_t *error) {
    fprintf(stderr, "PARSER ERROR: %s\n", error->message);
}


void parser_error_create(parser_t *parser, token_t token, const char *format, ...) {
    if (parser->error_idx >= PARSER_MAX_ERROR_COUNT) {
        for (int i = 0; i < parser->error_idx; i++) {
            parser_error_print(parser->errors[i]);
        }

        fprintf(stderr, "\nToo many parsing errors.\n");
        exit(1);
    }
    va_list args;
    va_start(args, format);

    parser_error_t *error = malloc(sizeof(parser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = malloc(msg_len);

    // Format the message
    vsnprintf(error->message, msg_len, format, args);

    va_end(args);

    error->token = token;
    parser->errors[parser->error_idx++] = error;
}

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
        parser_error_create(parser, current, "Expected token `%s` but got `%s`", token_type_to_string(token_type), token_type_to_string(current.type));
    }
    return current;
}

token_t parser_eat_and_expect(parser_t *parser, token_type_t token_type) {
    token_t current = parser_current(parser);
    if (current.type != token_type) {
        parser_error_create(parser, current, "Expected token `%s` but got `%s`", token_type_to_string(token_type), token_type_to_string(current.type));
        return current;
    }
    parser_eat(parser);
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


precedence_t get_precedence(token_type_t token_type) {
    switch (token_type) {
        case TOKEN_EQEQ:
            return PRECEDENCE_EQUALS;
        case TOKEN_LT:
        case TOKEN_LTE:
        case TOKEN_GT:
        case TOKEN_GTE:
            return PRECEDENCE_LGT;
        case TOKEN_COLON:
        case TOKEN_BANG:
            return PRECEDENCE_PREFIX;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return PRECEDENCE_SUM;
        case TOKEN_SLASH:
        case TOKEN_ASTERISK:
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
        case TOKEN_L_PAREN:
            parser_eat(parser);
            expr = parser_parse_expression(parser, PRECEDENCE_LOWEST);
            parser_eat_and_expect(parser, TOKEN_R_PAREN);
            break;
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
    }
    return expr;
}

expression_t* parser_parse_infix_expression(parser_t *parser, expression_t *left) {
    expression_t *expr = malloc(sizeof(expression_t));
    switch (parser_current(parser).type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_SLASH:
        case TOKEN_ASTERISK:
        case TOKEN_GT:
        case TOKEN_LT:
        case TOKEN_GTE:
        case TOKEN_LTE:
        case TOKEN_EQEQ: {
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
    }
    return expr;
}

expression_t* parser_parse_expression(parser_t *parser, precedence_t precedence) {
    expression_t *expr = parser_parse_prefix_expression(parser);
    if (expr == NULL) {
        return NULL;
    }

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
            default: {
                expression_t *exp = parser_parse_expression(parser, PRECEDENCE_LOWEST);

                if (exp == NULL) {
                    parser_error_create(parser, current, "Don't know how to parse this `%s`", token_type_to_string(current.type));
                    break;
                }

                program_add_statement(&parser->program, (statement_t) {
                    .type = STATEMENT_TYPE_EXPRESSION,
                    .data.expression = *exp
                });
            }
        }
        parser_eat(parser);
    }
}

program_t parse(token_t *tokens) {
    parser_t parser = (parser_t) {
        .index = 0,
        .tokens = tokens,
        .program = program_create(),
        // .errors = {NULL},
        .error_idx = 0
    };

    parser_parse(&parser);

    for (int i = 0; i < parser.program.statement_count; i++) {
        printf("\nSTATEMENT #%d\n", i);
        statement_debug(&parser.program.statements[i], 0);
        printf("\n");
    }

    if (parser.error_idx > 0) {
        for (int i = 0; i < parser.error_idx; i++) {
            parser_error_print(parser.errors[i]);
        }
    }
    return parser.program;
}
