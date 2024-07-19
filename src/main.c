#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef enum {
    TOKEN_L_BRACKET = '{',
    TOKEN_R_BRACKET = '}',
    TOKEN_L_PAREN = '(',
    TOKEN_R_PAREN = ')',
    TOKEN_WHITESPACE = ' ',
    TOKEN_SEMICOLON = ';',
    TOKEN_NEW_LINE = '\n',
    TOKEN_IDENTIFIER,
    TOKEN_DEFER,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} token_type_t;

typedef union {
    char* raw_str;
    char raw_char;
} token_value_t;

typedef struct {
    token_type_t type;
    int position;
    token_value_t value;
} token_t;

token_t token_new_mult_char(token_type_t type, int position, char* raw) {
    return (token_t) {
        .type = type,
        .position = position,
        .value = (token_value_t) {
            .raw_str = raw
        }
    };
}

token_t token_new_from_buffer(token_type_t type, int position, char* buffer, size_t buffer_length) {
    return (token_t) {
        .type = type,
        .position = position,
        .value = (token_value_t) {
            .raw_str = strdup(buffer)
        }
    };
}

token_t token_new_single_char(token_type_t type, int position, char raw) {
    return (token_t) {
        .type = type,
        .position = position,
        .value = (token_value_t) {
            .raw_char = raw
        }
    };
}

void token_debug(token_t token) {
    switch (token.type) {
        case TOKEN_L_BRACKET:
        case TOKEN_R_BRACKET:
        case TOKEN_L_PAREN:
        case TOKEN_R_PAREN:
        case TOKEN_WHITESPACE:
        case TOKEN_SEMICOLON:
            printf("Token '%c'", token.value.raw_char);
            break;
        case TOKEN_NEW_LINE:
            printf("Token NEW_LINE");
            break;
        case TOKEN_DEFER:
            printf("Token DEFER");
            break;
        case TOKEN_IDENTIFIER:
            printf("Token IDENTIFIER `%s`", token.value.raw_str);
            break;
        case TOKEN_UNKNOWN:
            printf("Token UNKNOWN `%s`", token.value.raw_str);
            break;
        case TOKEN_EOF:
            printf("Token EOF");
            break;
    }
}

const char* keywords[] = {
    "defer",
};

const token_type_t keyword_token_types[] = {
    TOKEN_DEFER,
};

#define ARRAY_SIZE(arr) (sizeof(arr))/(sizeof(arr[0]))
static_assert(ARRAY_SIZE(keywords) == ARRAY_SIZE(keyword_token_types),
    "keywords and keyword token types should have the same size");

token_type_t get_buffer_token_type(char* buffer) {
    for (int i = 0; i < ARRAY_SIZE(keywords); i++) {
        if (strcmp(keywords[i], buffer) == 0) {
            return keyword_token_types[i];
        }
    }
    // if token is identifier?
    return TOKEN_IDENTIFIER;
    // return TOKEN_UNKNOWN;
}

#define TOKENISER_BUFFER_SIZE 1024

int tokenise(const char* data, int data_length, token_t* tokens) {
    int buffer_index = 0;
    char buffer[TOKENISER_BUFFER_SIZE];
    int token_index = 0;

    for (int i = 0; i < data_length; i++) {
        char c = data[i];
        switch (c) {
            case TOKEN_NEW_LINE:
            case TOKEN_WHITESPACE:
            case TOKEN_L_BRACKET:
            case TOKEN_R_BRACKET:
            case TOKEN_L_PAREN:
            case TOKEN_R_PAREN:
            case TOKEN_SEMICOLON:
            {
                // commit buffer
                if (buffer_index > 0) {
                    buffer[buffer_index] = '\0';
                    token_type_t type = get_buffer_token_type(buffer);
                    tokens[token_index++] = token_new_from_buffer(type, i, buffer, buffer_index);
                    buffer_index = 0;
                }

                tokens[token_index++] = token_new_single_char(c, i, c);
                break;
            }
            default: {
                buffer[buffer_index++] = c;
            }
        }
    }

    tokens[token_index++] = (token_t) {.type = TOKEN_EOF };
    return token_index;
}

typedef struct {
    int index;
    token_t* tokens;
    program_t program;
} parser_t;

token_t parser_eat(parser_t *parser) {
    return parser->tokens[parser->index++];
}

token_t parser_peek(parser_t *parser) {
    return parser->tokens[parser->index];
}

void parser_parse_expression(parser_t *parser) {
}

void parser_parse_statement(parser_t *parser) {
}

void parser_parse_block(parser_t *parser) {
}

void parser_parse(parser_t *parser) {
    token_t current;
    while (current = parser_eat(parser), current.type != TOKEN_EOF) {
        token_debug(current);
        printf("\n");

        // if (current.type == TOKEN_L_BRACKET) {
        //     parser_parse_block(parser);
        // }
    }
    printf("\nEOF\n");
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
