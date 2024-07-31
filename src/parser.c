#include "parser.h"
#include "arena.h"
#include "ast.h"
#include "hashmap.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <stdbool.h>
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

    parser_error_t *error = arena_alloc(&parser->ctx.arena, sizeof(parser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = arena_alloc(&parser->ctx.arena, msg_len);

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
    printf("\n\n");
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

bool parser_optional_eat(parser_t *parser, token_type_t token_type) {
    token_t current = parser_current(parser);
    if (current.type != token_type) {
        return false;
    }
    parser_eat(parser);
    return true;
}

token_t parser_eat_and_expect(parser_t *parser, token_type_t token_type) {
    token_t current = parser_current(parser);
    // printf("\n-- Expecting:: %s ", token_type_to_string(token_type));
    // parser_debug(parser, "GOT ");
    // printf("\n -- \n");
    if (current.type != token_type) {
        printf("\n(!) Expected token `%s` but got `%s`\n", token_type_to_string(token_type), token_type_to_string(current.type));
        parser_error_create(parser, current, "Expected token `%s` but got `%s`", token_type_to_string(token_type), token_type_to_string(current.type));
        return current;
    }
    parser_eat(parser);
    return current;
}

literal_expression_t parser_parse_literal(parser_t *parser) {
    token_t current = parser_eat_and_expect(parser, TOKEN_LITERAL);
    const char first = current.value.raw_str[0];
    if (first == '\'') {
        return (literal_expression_t) {
            .type = LITERAL_TYPE_CHARACTER,
            .data = {
                .character = current.value.raw_str[1]
            }
        };
    } else if (first == '"') {
        return (literal_expression_t) {
            .type = LITERAL_TYPE_STRING,
            .data = {
                .string = arena_strdup(&parser->ctx.arena, current.value.raw_str)
            }
        };
    }

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


ptype_t parser_parse_type_hint(parser_t *parser) {
    token_t current = parser_current(parser);

    // if (current.type != TOKEN_COLON) return PTYPE_UNKNOWN;
    // parser_eat_and_expect(parser, TOKEN_COLON);

    ptype_t primitive = str_to_primitive_type(current.value.raw_str);
    if (primitive != PTYPE_UNKNOWN) {
        token_t type = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);
    }
    return primitive;
}

expression_t* parser_parse_expression(parser_t *parser, precedence_t precedence);
statement_t* parser_parse_statement(parser_t *parser);

vector_t parser_parse_comma_seperated_args(parser_t *parser) {
    vector_t args = vector_new(8, sizeof(expression_t));
    expression_t *arg = parser_parse_expression(parser, PRECEDENCE_LOWEST);

    while (arg != NULL) {
        vector_push(&args, arg);
        if (parser_current(parser).type != TOKEN_COMMA) break;
        parser_eat_and_expect(parser, TOKEN_COMMA);
        parser_debug(parser, "here\n");
        arg = parser_parse_expression(parser, PRECEDENCE_LOWEST);
    }
    parser_optional_eat(parser, TOKEN_COMMA); // allow trailing comma
    return args;
}

vector_t parser_parse_comma_seperated_params(parser_t *parser) {
    vector_t args = vector_new(8, sizeof(expression_t));
    do {
        if (parser_current(parser).type != TOKEN_IDENTIFIER) break;
        ptype_t ptype = parser_parse_type_hint(parser);

        if (ptype == PTYPE_UNKNOWN) {
            parser_error_create(parser, parser_current(parser), "Unrecognised type `%s`", parser_current(parser).value.raw_str);
            parser_eat(parser);
        }

        token_t identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);
        // printf("\n--> ");
        // token_debug(identifier);
        // printf("\n---\n");


        identifier_expression_t *param = arena_alloc(&parser->ctx.arena, sizeof(identifier_expression_t));
        param->name = identifier.value.raw_str,
        param->type = ptype;

        vector_push(&args, param);
    } while (parser_optional_eat(parser, TOKEN_COMMA));

    return args;
}


expression_t* parser_parse_prefix_expression(parser_t *parser) {
    // parse prefix
    switch (parser_current(parser).type) {
        case TOKEN_L_BRACE: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_BLOCK;
            expr->data.block = block_expression_new();
            parser_eat_and_expect(parser, TOKEN_L_BRACE);

            while (parser_current(parser).type != TOKEN_R_BRACE) {
                // parser_debug(parser, "\nbefore block parsing\n");
                block_add_statement(&expr->data.block,
                    parser_parse_statement(parser)
                );
                // parser_debug(parser, "\nafter block parsing\n");
            };
            // parser_debug(parser, "\nafter while parsing\n");
            parser_eat_and_expect(parser, TOKEN_R_BRACE);
            return expr;
        }
        case TOKEN_L_PAREN: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            parser_eat_and_expect(parser, TOKEN_L_PAREN);
            expr = parser_parse_expression(parser, PRECEDENCE_LOWEST);
            parser_eat_and_expect(parser, TOKEN_R_PAREN);
            return expr;
        }
        case TOKEN_LITERAL: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_LITERAL;
            expr->data.literal = parser_parse_literal(parser);
            return expr;
        }
        case TOKEN_PLUS:
        case TOKEN_BANG:
        case TOKEN_MINUS: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_PREFIX;
            expr->data.prefix = (prefix_expression_t) {
                .operand = parser_eat(parser),
                .right = parser_parse_expression(parser, PRECEDENCE_PREFIX)
            };
            return expr;
        }
        case TOKEN_IDENTIFIER: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            char* identifier_name = arena_strdup(&parser->ctx.arena, parser_eat(parser).value.raw_str);

            if (parser_current(parser).type == TOKEN_L_PAREN) {
                parser_eat_and_expect(parser, TOKEN_L_PAREN);
                vector_t args = parser_parse_comma_seperated_args(parser);
                parser_eat_and_expect(parser, TOKEN_R_PAREN);

                expr->type =
                    identifier_name[0] == '@' ?
                    EXPRESSION_TYPE_STATIC_CALL :
                    EXPRESSION_TYPE_CALL;

                expr->data.call = (call_expression_t) {
                    .identifier_name = identifier_name,
                    .arguments = args
                };
                return expr;
            }

            expr->type = EXPRESSION_TYPE_IDENTIFIER;
            expr->data.identifier.name = identifier_name;
            printf("=========> %s\n", expr->data.identifier.name);
            return expr;
        }
        case TOKEN_FN: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            parser_eat(parser);
            ptype_t type = parser_parse_type_hint(parser);
            printf("\n parser type hint for function is : %d\n", type);

            token_t identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);
            char* function_name = identifier.value.raw_str;
            // printf("\n function name is %s \n", function_name);
            parser_eat_and_expect(parser, TOKEN_L_PAREN);

            vector_t params = parser_parse_comma_seperated_params(parser);
            parser_eat_and_expect(parser, TOKEN_R_PAREN);


            expr->type = EXPRESSION_TYPE_FUNC_DECL;
            expr->data.func_decl = (func_decl_expression_t) {
                .params = params,
                .name = arena_strdup(&parser->ctx.arena, function_name),
                .value = parser_parse_expression(parser, PRECEDENCE_CALL),
                .type = type
            };
            return expr;
        }
        case TOKEN_RETURN: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            parser_eat_and_expect(parser, TOKEN_RETURN);
            expr->type = EXPRESSION_TYPE_RETURN;
            expr->data.return_exp = (return_expression_t) {
                .expression = parser_parse_expression(parser, PRECEDENCE_LOWEST)
            };
            return expr;
        }
        case TOKEN_IF: {
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            parser_eat_and_expect(parser, TOKEN_IF);
            expr->type = EXPRESSION_TYPE_CONDITIONAL;
            expr->data.conditional.predicate = parser_parse_expression(parser, PRECEDENCE_LOWEST);
            parser_expect(parser, TOKEN_L_BRACE);
            expr->data.conditional.success_branch = parser_parse_expression(parser, PRECEDENCE_LOWEST);
            expr->data.conditional.fail_branch = NULL;
            if (parser_current(parser).type == TOKEN_ELSE) {
                parser_eat_and_expect(parser, TOKEN_ELSE);
                expr->data.conditional.fail_branch = parser_parse_expression(parser, PRECEDENCE_LOWEST);
            }
            return expr;
        }
        default:
            parser_debug(parser, "hit NULL case here with token \n");
            return NULL;
    }
    return NULL;
}

expression_t* parser_parse_infix_expression(parser_t *parser, expression_t *left) {

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
            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_INFIX;
            token_t operand = parser_eat(parser);
            precedence_t precedence = get_precedence(operand.type);
            expr->data.infix = (infix_expression_t) {
                .operand = operand,
                .right = parser_parse_expression(parser, precedence),
                .left = left
            };
            return expr;
        }
        // 5:to_i32
        case TOKEN_COLON: {
            parser_eat_and_expect(parser, TOKEN_COLON);
            expression_t *right = parser_parse_expression(parser, PRECEDENCE_CALL);
            assert(right->type == EXPRESSION_TYPE_CALL);
            vector_insert(&right->data.call.arguments, 0, left);
            return right;
        }
        default:
            return NULL;
    }
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

    // parser_debug(parser, "\nafter expression parsing: ");
    if (parser_current(parser).type == TOKEN_SEMICOLON) {
        // printf("\n EAT SEMIC COLON AFTER EXPR\n");
        parser_eat_and_expect(parser, TOKEN_SEMICOLON);
    }
    if (parser_current(parser).type == TOKEN_NEW_LINE) {
        // printf("\n EAT NEW LINE AFTER EXPR\n");
        parser_eat_and_expect(parser, TOKEN_NEW_LINE);
    }
    return expr;
}

statement_t* parser_parse_statement(parser_t *parser) {
    statement_t *s = arena_alloc(&parser->ctx.arena, sizeof(statement_t));
    token_t current;
    while (current = parser_current(parser), current.type == TOKEN_NEW_LINE || current.type == TOKEN_SEMICOLON) {
        parser_eat(parser);
    }

    switch (current.type) {
        case TOKEN_LET: {
            token_t let = parser_eat(parser);
            ptype_t type = parser_parse_type_hint(parser);
            token_t identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);

            token_t eq = parser_eat_and_expect(parser, TOKEN_EQ);

            identifier_expression_t idexpr = (identifier_expression_t) {
                .name = arena_strdup(&parser->ctx.arena, identifier.value.raw_str),
                .value = parser_parse_expression(parser, PRECEDENCE_LOWEST),
                .type = type
            };

            s->type = STATEMENT_TYPE_LET;
            s->data.let.identifier = idexpr;
            return s;
        }
        case TOKEN_EXTERN: {
            parser_eat_and_expect(parser, TOKEN_EXTERN);
            parser_eat_and_expect(parser, TOKEN_FN); // only fn can be external
            ptype_t type = parser_parse_type_hint(parser);
            if (type == PTYPE_UNKNOWN) {
                parser_error_create(parser, parser_current(parser), "External functional need to define return type");
            }
            token_t fn_identifier = parser_eat_and_expect(parser, TOKEN_IDENTIFIER);

            parser_eat_and_expect(parser, TOKEN_L_PAREN);
            vector_t params = parser_parse_comma_seperated_params(parser);
            parser_eat_and_expect(parser, TOKEN_R_PAREN);

            expression_t *expr = arena_alloc(&parser->ctx.arena, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_FUNC_DECL;
            expr->data.func_decl = (func_decl_expression_t) {
                .params = params,
                .name = arena_strdup(&parser->ctx.arena,  fn_identifier.value.raw_str),
                .value = NULL,
                .type = type
            };

            s->type = STATEMENT_TYPE_EXPRESSION;
            s->data.expression = *expr;
            return s;
        }
        default: {
            expression_t *exp = parser_parse_expression(parser, PRECEDENCE_LOWEST);

            if (exp == NULL) {
                parser_error_create(parser, current, "Don't know how to parse this `%s`", token_type_to_string(current.type));
                break;
            }

            s->type = STATEMENT_TYPE_EXPRESSION;
            s->data.expression = *exp;
            return s;
        }
    }
    parser_debug(parser, "failed at this token. case not covered");
    exit(0);
}

void parser_parse(parser_t *parser) {
    token_t current;
    while (current = parser_current(parser), current.type != TOKEN_EOF) {
        statement_t *s = parser_parse_statement(parser);
        vector_push_ptr(&parser->program.statements, s);
        // program_add_statement(&parser->program, s);
    }
}

parser_t parser_new() {
    return (parser_t) {
        .index = 0,
        .program = program_create(),
        .error_idx = 0,
        .ctx = (ParserContext) {
            .arena = arena_new(1024*1024)
        }
    };
}

void parser_destroy(parser_t *parser) {
    printf("[Parser Stats] Arena contained %ld bytes out of total %ld bytes (%ld%%)\n", parser->ctx.arena.offset, parser->ctx.arena.capacity, 100*parser->ctx.arena.offset/parser->ctx.arena.capacity);
    arena_destroy(&parser->ctx.arena);
}


program_t parse(parser_t *parser, token_t *tokens) {
    parser->tokens = tokens;
    parser->index = 0;
    parser->error_idx = 0;
    parser->program = program_create();

    parser_parse(parser);

    for (int i = 0; i < parser->program.statements.count; i++) {
        printf("\nSTATEMENT #%d\n", i);
        statement_debug(vector_get(&parser->program.statements, i), 0);
        printf("\n");
    }

    printf("\n\n");
    if (parser->error_idx > 0) {
        for (int i = 0; i < parser->error_idx; i++) {
            parser_error_print(parser->errors[i]);
        }
    }
    return parser->program;
}
