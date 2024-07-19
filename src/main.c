#include <_ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokeniser.h"
#include "ast.h"

typedef struct {
    int length;
    const char* buffer;
} file_read_result_t;

file_read_result_t read_file_to_str(const char* filename) {
    char *buffer = 0;
    int length = -1;
    FILE *f = fopen(filename, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    }

    return (file_read_result_t) {
        .length = length,
        .buffer = buffer
    };
}

void file_read_result_free(file_read_result_t res) {
    free((void*) res.buffer);
}


typedef struct {
    int index;
    token_t* tokens;
    program_t program;
} parser_t;


token_t parser_peek(parser_t *parser) {
    return parser->tokens[parser->index+1];
}

token_t parser_current(parser_t *parser) {
    return parser->tokens[parser->index];
}

token_t parser_eat(parser_t *parser) {
    printf("\n::: ");;
    token_debug(parser_current(parser));
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

typedef enum {
    PRECEDENCE_LOWEST,
    PRECEDENCE_EQUALS, // ==
    PRECEDENCE_LGT, // > or <
    PRECEDENCE_SUM,
    PRECEDENCE_PROD,
    PRECEDENCE_PREFIX,
    PRECEDENCE_CALL
} precedence_t;

precedence_t get_precedence(token_type_t token_type) {
    switch (token_type) {
        case TOKEN_L_BRACKET:
        case TOKEN_R_BRACKET:
        case TOKEN_L_PAREN:
        case TOKEN_R_PAREN:
        case TOKEN_SEMICOLON:
        case TOKEN_COLON:
        case TOKEN_BANG:
        case TOKEN_SINGLE_QUOTE:
        case TOKEN_DOUBLE_QUOTE:
        case TOKEN_BACKTICK:
        case TOKEN_BACKSLASH:
        case TOKEN_NEW_LINE:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_DIV:
        case TOKEN_MULT:
        case TOKEN_EQ:
        case TOKEN_LITERAL:
        case TOKEN_IDENTIFIER:
        case TOKEN_DEFER:
        case TOKEN_IF:
        case TOKEN_ELSE:
        case TOKEN_LET:
        case TOKEN_MUT:
        case TOKEN_UNKNOWN:
        case TOKEN_EOF:
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
            printf("minuxs\n");
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

expression_t* parser_parse_infix_expression(parser_t *parser, expression_t left) {
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
            return NULL;
            // printf("\nInvalid token!\n");
            // token_debug(parser_peek(parser));
            // exit(1);
    }
    return expr;
}

void debug_expression(expression_t *expression) {
    switch (expression->type) {
    case EXPRESSION_TYPE_PREFIX:
        printf("expr PREFIX (");
        token_debug(expression->data.prefix.operand);
        printf(")\n      R: ");
        debug_expression(expression->data.prefix.right);
        break;
    case EXPRESSION_TYPE_INFIX:

        printf("expr INFIX (");
        token_debug(expression->data.infix.operand);
        printf(")\n      L: ");
        debug_expression(expression->data.infix.left);
        printf("\n      R: ");
        debug_expression(expression->data.infix.right);
        break;
    case EXPRESSION_TYPE_LITERAL:
        printf("expr LITERAL (%ld)", expression->data.literal.data.i64);
        break;
    case EXPRESSION_TYPE_IDENTIFIER:
        printf("expr IDENTIFIER (%s)", expression->data.identifier.name);;
        break;
    }
}

expression_t* parser_parse_expression(parser_t *parser, precedence_t precedence) {
    expression_t *expr = parser_parse_prefix_expression(parser);
    if (expr == NULL) return NULL;
    return expr;
}

void parser_parse_statement(parser_t *parser) {
}

void parser_parse_block(parser_t *parser) {
}

void parser_parse(parser_t *parser) {
    token_t current;
    statement_t statements[1024];
    int statements_index = 0;
    while (current = parser_current(parser), current.type != TOKEN_EOF) {

        switch (current.type) {
            default:
                statements[statements_index++] = (statement_t) {
                    .type = STATEMENT_TYPE_EXPRESSION,
                    .data.expression = *parser_parse_expression(parser, PRECEDENCE_LOWEST)
                };
        }
        parser_eat(parser);
    }
    printf("\nEOF\n");

    for (int i = 0; i < statements_index; i++) {
        debug_expression(&statements[i].data.expression);
        printf("\n");
    }
}

void parse(token_t *tokens) {
    parser_t parser = (parser_t) {
        .index = 0,
        .tokens = tokens,
        .program = program_create()
    };

    parser_parse(&parser);
}

#define MAX_TOKENS 4096
int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Argument mismatch.\nUsage: krama <input-file> <output-file>\n");
        return 1;
    }

    char* input_file = argv[1];
    char* output_file = argv[2];


    file_read_result_t result = read_file_to_str(input_file);

    if (result.length == -1) {
        printf("could not read file\n");
        return 1;
    }

    // TODO use dynamic array here
    token_t* tokens = malloc(MAX_TOKENS * sizeof(token_t));
    int token_count = tokenise(result.buffer, result.length, tokens);

    parse(tokens);

    free(tokens);
    file_read_result_free(result);
    return 0;
}
