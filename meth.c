#include "stdio.h"
#include "stdlib.h"

#define true 1
#define false 0
#define string char *

typedef enum
{
    ADD,
    MIN,
    DIV,
    MUL
} OpType;

typedef enum
{
    NUMBER,
    OP,
    OPEN_PAR,
    CLOSE_PAR
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

void tokenise(string filename, Token *tokens)
{
    FILE *fptr;
    fptr = fopen(filename, "r");

    int i = 0;
    char c;
    // Token *tokens = malloc(512 * sizeof(Token));

    while (c != -1)
    {
        c = fgetc(fptr);
        printf("%c", c);

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
            break;
        }

        i++;
    }
}

int main()
{
    printf("hello \n");

    Token *tokens = malloc(512 * sizeof(Token));
    tokenise("./expr.meth", tokens);
    return 0;
}
