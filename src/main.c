#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int length;
    const char* buffer;
} FileReadResult;

FileReadResult read_file_to_str(const char* filename) {
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

  return (FileReadResult) {
      .length=length,
      .buffer=buffer
  };
}

void FileReadResult_free(FileReadResult res) {
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
    TOKEN_DEFER,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} TokenType;

typedef union {
    char* raw_str;
    char raw_char;
} TokenValue;

typedef struct {
    TokenType type;
    int position;
    TokenValue value;
} Token;

Token Token_new_mult_char(TokenType type, int position, char* raw) {
    return (Token) {
        .type = type,
        .position = position,
        .value = (TokenValue) {
            .raw_str = raw
        }
    };
}

Token Token_new_from_buffer(TokenType type, int position, char* buffer, size_t bufferLength) {
    return (Token) {
        .type = type,
        .position = position,
        .value = (TokenValue) {
            .raw_str = strdup(buffer)
        }
    };
}

Token Token_new_single_char(TokenType type, int position, char raw) {
    return (Token) {
        .type = type,
        .position = position,
        .value = (TokenValue) {
            .raw_char = raw
        }
    };
}

void Token_debug(Token token) {
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

const TokenType keyword_token_types[] = {
    TOKEN_DEFER,
};

#define ARRAY_SIZE(arr) (sizeof(arr))/(sizeof(arr[0]))
static_assert(ARRAY_SIZE(keywords) == ARRAY_SIZE(keyword_token_types),
    "keywords and keyword token types should have the same size");

TokenType get_buffer_token_type(char* buffer) {
    for (int i = 0; i < ARRAY_SIZE(keywords); i++) {
        if (strcmp(keywords[i], buffer) == 0) {
            return keyword_token_types[i];
        }
    }
    return TOKEN_UNKNOWN;
}

#define TOKENISER_BUFFER_SIZE 1024

int tokenise(const char* data, int data_length, Token* tokens) {
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
                    TokenType type = get_buffer_token_type(buffer);
                    tokens[token_index++] = Token_new_from_buffer(type, i, buffer, buffer_index);
                    buffer_index = 0;
                }

                tokens[token_index++] = Token_new_single_char(c, i, c);
                break;
            }
            default: {
                buffer[buffer_index++] = c;
            }
        }
    }

    tokens[token_index++] = (Token) {.type = TOKEN_EOF };
    return token_index;
}

#define MAX_TOKENS 4096
int main() {
    FileReadResult result = read_file_to_str("src/main.c");

    if (result.length == -1) {
        printf("could not read file\n");
        return 1;
    }

    // TODO use dynamic array here
    Token* tokens = malloc(MAX_TOKENS * sizeof(Token));
    int token_count = tokenise(result.buffer, result.length, tokens);


    for (int i = 0; i < token_count; i ++) {
        if (tokens[i].type != TOKEN_WHITESPACE) {
            Token_debug(tokens[i]);
            printf("\n");
        }
    }

    free(tokens);
    FileReadResult_free(result);
    return 0;
}
