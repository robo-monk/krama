#include "parser.h"
#include "ast.h"
#include "hashmap.h"
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
    // printf("\n-- Expecting:: %s ", token_type_to_string(token_type));
    // parser_debug(parser, "GOT ");
    // printf("\n -- \n");
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
statement_t parser_parse_statement(parser_t *parser);

expression_t* parser_parse_prefix_expression(parser_t *parser) {
    expression_t *expr = malloc(sizeof(expression_t));
    // parse prefix
    switch (parser_current(parser).type) {
        case TOKEN_L_BRACE: {
            expr->type = EXPRESSION_TYPE_BLOCK;
            expr->data.block = block_expression_new();
            parser_eat_and_expect(parser, TOKEN_L_BRACE);
            while (parser_current(parser).type != TOKEN_R_BRACE) {
                parser_debug(parser, "\nbefore block parsing\n");
                block_add_statement(&expr->data.block,
                    parser_parse_statement(parser)
                );
                parser_debug(parser, "\nafter block parsing\n");
            };
            parser_debug(parser, "\nafter while parsing\n");
            parser_eat_and_expect(parser, TOKEN_R_BRACE);
            break;
        }
        case TOKEN_L_PAREN:
            parser_eat_and_expect(parser, TOKEN_L_PAREN);
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
            char* identifier_name = strdup(parser_eat(parser).value.raw_str);
            scope_entry_t *entry = hashmap_get(parser->scope.table, identifier_name);
            if (entry == NULL) {
                parser_error_create(parser, parser_current(parser), "identifier `%s` is not declared.", identifier_name);
                break;
            }
            printf("\n table returns: %s \n", entry->identifier.name);
            expr->type = EXPRESSION_TYPE_IDENTIFIER;
            expr->data.identifier = (identifier_expression_t) {
                .name = identifier_name
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
    parser_debug(parser, "\nafter expression parsing: ");
    if (parser_current(parser).type == TOKEN_SEMICOLON) {
        printf("\n EAT SEMIC COLON AFTER EXPR\n");
        parser_eat_and_expect(parser, TOKEN_SEMICOLON);
    }
    if (parser_current(parser).type == TOKEN_NEW_LINE) {
        printf("\n EAT NEW LINE AFTER EXPR\n");
        parser_eat_and_expect(parser, TOKEN_NEW_LINE);
    }
    return expr;
}

void scope_define_let(scope_t *scope, scope_entry_t entry) {
    scope_entry_t *ptr = (scope_entry_t*) malloc(sizeof(entry));
    ptr->identifier = entry.identifier;
    hashmap_insert(scope->table, entry.identifier.name, ptr);
}

statement_t parser_parse_statement(parser_t *parser) {
    token_t current;
    while (current = parser_current(parser), current.type == TOKEN_NEW_LINE || current.type == TOKEN_SEMICOLON) {
        parser_eat(parser);
    }

    switch (current.type) {
        case TOKEN_SEMICOLON:
        case TOKEN_NEW_LINE:
            printf("\n unreachable?\n");
            break;
        case TOKEN_LET: {
            token_t let = parser_eat(parser);
            token_t identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);
            token_t eq = parser_eat_and_expect(parser, TOKEN_EQ);
            identifier_expression_t idexpr = (identifier_expression_t) {
                .name = strdup(identifier.value.raw_str),
                .value = parser_parse_expression(parser, PRECEDENCE_LOWEST)
            };
            scope_entry_t *entry = hashmap_get(parser->scope.table, idexpr.name);

            if (entry != NULL) {
                parser_error_create(parser, current, "Identifier `%s` has already been declared.", idexpr.name);
            }

            scope_define_let(&parser->scope, (scope_entry_t) {
                .identifier = idexpr
            });
            return (statement_t) {
                .type = STATEMENT_TYPE_LET,
                .data.let = {
                    .identifier = idexpr
                }
            };
            break;
        }
        default: {
            expression_t *exp = parser_parse_expression(parser, PRECEDENCE_LOWEST);

            if (exp == NULL) {
                parser_error_create(parser, current, "Don't know how to parse this `%s`", token_type_to_string(current.type));
                break;
            }

            return (statement_t) {
                .type = STATEMENT_TYPE_EXPRESSION,
                .data.expression = *exp
            };
        }
    }
    printf("\nUnrecoverable error\n");
    exit(0);
}

void parser_parse(parser_t *parser) {
    token_t current;
    while (current = parser_current(parser), current.type != TOKEN_EOF) {
        program_add_statement(&parser->program, parser_parse_statement(parser));
        parser_debug(parser, "---- after adding statement ---");
        // parser_eat(parser);
    }
}

void free_scope_entry(void* f) {
    free(f);
}

parser_t create_parser() {
    return (parser_t) {
        .index = 0,
        .program = program_create(),
        .scope = {
            .table = hashmap_create(free_scope_entry),
            .upper = NULL
        },
        .error_idx = 0
    };
}
program_t parse(parser_t *parser, token_t *tokens) {
    parser->tokens = tokens;
    parser->index = 0;
    parser->error_idx = 0;
    parser->program = program_create();

    parser_parse(parser);

    for (int i = 0; i < parser->program.statement_count; i++) {
        printf("\nSTATEMENT #%d\n", i);
        statement_debug(&parser->program.statements[i], 0);
        printf("\n");
    }

    if (parser->error_idx > 0) {
        for (int i = 0; i < parser->error_idx; i++) {
            parser_error_print(parser->errors[i]);
        }
    }
    return parser->program;
}
