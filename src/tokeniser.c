#include "tokeniser.h"
#include <_ctype.h>
#include <ctype.h>

const char* tokeniser_keywords[] = {
    "defer",
};

const token_type_t tokeniser_keyword_token_types[] = {
    TOKEN_DEFER,
};


#define ARRAY_SIZE(arr) (sizeof(arr))/(sizeof(arr[0]))
static_assert(ARRAY_SIZE(tokeniser_keywords) == ARRAY_SIZE(tokeniser_keyword_token_types),
    "keywords and keyword token types should have the same size");




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
        case TOKEN_SEMICOLON:
        case TOKEN_COLON:
        case TOKEN_SINGLE_QUOTE :
        case TOKEN_DOUBLE_QUOTE:
        case TOKEN_BACKTICK:
        case TOKEN_BACKSLASH :
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_DIV:
        case TOKEN_MULT:
        case TOKEN_EQ:
            printf("Token '%c'", token.value.raw_char);
            break;
        case TOKEN_NEW_LINE:
            printf("Token NEW_LINE");
            break;
        case TOKEN_DEFER:
            printf("Token DEFER");
            break;
        case TOKEN_LITERAL:
            printf("Token LITERAL `%s`", token.value.raw_str);
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
        case TOKEN_IF:
            printf("Token IF");
            break;
        case TOKEN_ELSE:
            printf("Token ELSE");
            break;
        case TOKEN_LET:
            printf("Token LET");
            break;
        case TOKEN_MUT:
            printf("Token MUT");
            break;
    }
}


token_type_t get_buffer_token_type(char* buffer) {
    for (int i = 0; i < ARRAY_SIZE(tokeniser_keywords); i++) {
        if (strcmp(tokeniser_keywords[i], buffer) == 0) {
            return tokeniser_keyword_token_types[i];
        }
    }
    // if token is identifier?
    if (isdigit(buffer[0])) {
        return TOKEN_LITERAL;
    }

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
            case ' ':
            case '\t':
            case '\f':
            case '\v':
                // Space (0x20, ' '),
                // Form feed (0x0c, '\f'),
                // Line feed (0x0a, '\n'),
                // Carriage return (0x0d, '\r'),
                // Horizontal tab (0x09, '\t'),
                // Vertical tab (0x0b, '\v'),
                // commit buffer
                if (buffer_index > 0) {
                    buffer[buffer_index] = '\0';
                    token_type_t type = get_buffer_token_type(buffer);
                    tokens[token_index++] = token_new_from_buffer(type, i, buffer, buffer_index);
                    buffer_index = 0;
                }
                break;
            case TOKEN_NEW_LINE:
            case TOKEN_L_BRACKET:
            case TOKEN_R_BRACKET:
            case TOKEN_L_PAREN:
            case TOKEN_R_PAREN:
            case TOKEN_SEMICOLON:
            case TOKEN_BANG:
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_MULT:
            case TOKEN_DIV:
            case TOKEN_EQ:
            {
                // commit buffer
                if (buffer_index > 0) {
                    buffer[buffer_index] = '\0';
                    token_type_t type = get_buffer_token_type(buffer);
                    tokens[token_index++] = token_new_from_buffer(type, i, buffer, buffer_index);
                    buffer_index = 0;
                }

                tokens[token_index++] = token_new_single_char((token_type_t) c, i, c);
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
