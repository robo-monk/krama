#include "tokeniser.h"
#include <_ctype.h>
#include <ctype.h>

const char* tokeniser_keywords[] = {
    "defer",
    "let",
    "mut",
    "if",
    "else",
    "return",
    "def",
    "for",
    "loop",
};

const token_type_t tokeniser_keyword_token_types[] = {
    TOKEN_DEFER,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_DEF,
    TOKEN_FOR,
    TOKEN_LOOP
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

const char* token_type_to_string(token_type_t type) {
    switch (type) {
        case TOKEN_L_BRACE:    return "{";
        case TOKEN_R_BRACE:    return "}";
        case TOKEN_L_BRACKET:    return "[";
        case TOKEN_R_BRACKET:    return "]";
        case TOKEN_L_PAREN:      return "(";
        case TOKEN_R_PAREN:      return ")";
        case TOKEN_COMMA:       return ",";
        case TOKEN_SEMICOLON:    return ";";
        case TOKEN_COLON:        return ":";
        case TOKEN_SINGLE_QUOTE: return "'";
        case TOKEN_DOUBLE_QUOTE: return "\"";
        case TOKEN_BACKTICK:     return "`";
        case TOKEN_BACKSLASH:    return "\\";
        case TOKEN_PLUS:         return "+";
        case TOKEN_MINUS:        return "-";
        case TOKEN_SLASH:        return "/";
        case TOKEN_ASTERISK:     return "*";
        case TOKEN_GT:           return ">";
        case TOKEN_LT:           return "<";
        case TOKEN_EQ:           return "=";
        case TOKEN_BANG:         return "!";
        case TOKEN_NEW_LINE:     return "NEW_LINE";
        case TOKEN_DEFER:        return "DEFER";
        case TOKEN_LITERAL:      return "LITERAL";
        case TOKEN_IDENTIFIER:   return "IDENTIFIER";
        case TOKEN_UNKNOWN:      return "UNKNOWN";
        case TOKEN_EOF:          return "EOF";
        case TOKEN_IF:           return "IF";
        case TOKEN_ELSE:         return "ELSE";
        case TOKEN_LET:          return "LET";
        case TOKEN_MUT:          return "MUT";
        case TOKEN_GTE:          return ">=";
        case TOKEN_LTE:          return "<=";
        case TOKEN_NEQ:          return "!=";
        case TOKEN_EQEQ:         return "==";
        case TOKEN_ATOM:         return "ATOM";
        case TOKEN_RETURN:       return "RETURN";
        case TOKEN_DEF:          return "DEF";
        case TOKEN_FOR:          return "FOR";
        case TOKEN_LOOP:         return "LOOP";
        default:                 return "INVALID_TOKEN_TYPE";
    }
}

void token_debug(token_t token) {
    switch (token.type) {
        case TOKEN_LITERAL:
            printf("Token LITERAL `%s`", token.value.raw_str);
            break;
        case TOKEN_IDENTIFIER:
            printf("Token IDENTIFIER `%s`", token.value.raw_str);
            break;
        case TOKEN_UNKNOWN:
            printf("Token UNKNOWN `%s`", token.value.raw_str);
            break;
        default:
            printf("Token %s", token_type_to_string(token.type));
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

            case TOKEN_LT:
                if (data[i+1] == TOKEN_EQ) {
                    tokens[token_index++] = token_new_mult_char(TOKEN_LTE, i, "<=");
                    i++;
                    break;
                }
            case TOKEN_GT:
                if (data[i+1] == TOKEN_EQ) {
                    tokens[token_index++] = token_new_mult_char(TOKEN_GTE, i, ">=");
                    i++;
                    break;
                }
            case TOKEN_BANG:
                if (data[i+1] == TOKEN_EQ) {
                    tokens[token_index++] = token_new_mult_char(TOKEN_NEQ, i, "!=");
                    i++;
                    break;
                }
            case TOKEN_EQ:
                if (data[i+1] == TOKEN_EQ) {
                    tokens[token_index++] = token_new_mult_char(TOKEN_EQEQ, i, "!=");
                    i++;
                    break;
                }
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_ASTERISK:
            case TOKEN_SLASH:
            case TOKEN_NEW_LINE:
            case TOKEN_L_BRACE:
            case TOKEN_R_BRACE:
            case TOKEN_L_BRACKET:
            case TOKEN_R_BRACKET:
            case TOKEN_L_PAREN:
            case TOKEN_R_PAREN:
            case TOKEN_SEMICOLON:
            case TOKEN_COMMA:
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
