#include "stdio.h"
#include "stdlib.h"

#define bool int
#define true 1
#define false 0
#define string char *

typedef enum
{
    ADD='+',
    MIN='-',
    DIV='/',
    MUL='*'
} OpType;

typedef enum
{
    NUMBER,
    OP,
    OPEN_PAR='(',
    CLOSE_PAR=')',
    PROGRAM_END
} TokenType;

typedef union
{
    int n;
    OpType op_type;
} TokenValue;

typedef struct
{
    TokenType type;
    TokenValue value;
} Token;

Token new_op_token(TokenType type, OpType op_type)
{
    Token t = {
        .type = type,
        .value = {
            .op_type = op_type}};
    return t;
}

Token new_token(TokenType type)
{
    Token t = {
        .type = type};
    return t;
}

int parse_number(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

void tokenise(string filename, Token *tokens)
{
    FILE *fptr;
    fptr = fopen(filename, "r");

    int i = -1;
    char c;
    Token current_token;
    // Token *tokens = malloc(512 * sizeof(Token));
    bool is_constructing_multichar_token = false;

    while (c != -1)
    {
        i++;

        c = fgetc(fptr);
        int number = parse_number(c);

        if (number > -1) {
            if (!is_constructing_multichar_token) {
                is_constructing_multichar_token = true;
                current_token = new_token(NUMBER);
                current_token.value.n = number;
            } else {
                int current_token_value = current_token.value.n;
                current_token_value <<= 1;
                current_token_value += number;
                current_token.value.n = current_token_value;
            }
            continue;
        }

        // done with construction
        if (is_constructing_multichar_token) {
            tokens[i] = current_token;
            i++;
            is_constructing_multichar_token = false;
        }

        switch (c)
        {
        case '+':
            tokens[i] = new_op_token(OP, ADD);
            break;
        case '-':
            tokens[i] = new_op_token(OP, MIN);
            break;
        case '/':
            tokens[i] = new_op_token(OP, DIV);
            break;
        case '*':
            tokens[i] = new_op_token(OP, MUL);
            break;
        case '(':
            tokens[i] = new_token(OPEN_PAR);
            break;
        case ')':
            tokens[i] = new_token(CLOSE_PAR);
            break;
        default:
            if (number > -1) {
                // printf("%d ", number);
            }
            break;
        }
    }

    if (is_constructing_multichar_token) {
        tokens[i] = current_token;
        i++;
        is_constructing_multichar_token = false;
    }
    tokens[i] = new_token(PROGRAM_END);
}

void dbg_tokens(Token* tokens) {
    int i = 0;
    while (tokens[i].type != PROGRAM_END) {
        Token current_token = tokens[i];
        if (current_token.type == OP) {
            printf("Operand: %c \n", current_token.value.op_type);
        } else if (current_token.type == NUMBER) {
            printf("Number: %d \n", current_token.value.n);
        } else {
            printf("Token: %c \n", current_token.type);
        }
        i++;
    }
}

int main()
{
    Token *tokens = malloc(512 * sizeof(Token));
    tokenise("./expr.meth", tokens);
    dbg_tokens(tokens);
    return 0;
}
