#include "stdio.h"
#include "stdlib.h"

#define bool int
#define true 1
#define false 0
#define string char *

typedef enum
{
    ADD = '+',
    MIN = '-',
    DIV = '/',
    MUL = '*'
} OpType;

typedef enum
{
    NUMBER,
    OP,
    OPEN_PAR = '(',
    CLOSE_PAR = ')',
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

int parse_number(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    return -1;
}

void tokenise(string filename, Token *tokens)
{
    FILE *fptr;
    fptr = fopen(filename, "r");

    int i = 0;
    int number;
    char c = 0;
    Token current_token;
    bool is_constructing_multichar_token = false;

    while ((c = fgetc(fptr)) != EOF)
    {
        if ((number = parse_number(c)) > -1)
        {
            if (!is_constructing_multichar_token)
            {
                is_constructing_multichar_token = true;
                current_token = new_token(NUMBER);
                current_token.value.n = number;
            }
            else
            {
                current_token.value.n = (current_token.value.n * 10) + number;
            }
            continue;
        }

        // done with construction
        if (is_constructing_multichar_token)
        {
            tokens[i++] = current_token;
            is_constructing_multichar_token = false;
        }

        switch (c)
        {
        case '+':
            tokens[i++] = new_op_token(OP, ADD);
            break;
        case '-':
            tokens[i++] = new_op_token(OP, MIN);
            break;
        case '/':
            tokens[i++] = new_op_token(OP, DIV);
            break;
        case '*':
            tokens[i++] = new_op_token(OP, MUL);
            break;
        case '(':
            tokens[i++] = new_token(OPEN_PAR);
            break;
        case ')':
            tokens[i++] = new_token(CLOSE_PAR);
            break;
        default:
            break;
        }
    }

    if (is_constructing_multichar_token)
    {
        tokens[i++] = current_token;
    }

    tokens[i] = new_token(PROGRAM_END);
    fclose(fptr);
}

void dbg_token(Token token)
{
    if (token.type == OP)
    {
        printf("Operand: %c \n", token.value.op_type);
    }
    else if (token.type == NUMBER)
    {
        printf("Number: %d \n", token.value.n);
    }
    else
    {
        printf("Token: %c \n", token.type);
    }
}
void dbg_tokens(Token *tokens)
{
    int i = 0;
    while (tokens[i].type != PROGRAM_END)
    {
        Token current_token = tokens[i++];
        dbg_token(current_token);
    }
}

// produces program that when run
void compile_c_to_str(Token *tokens, string program)
{
    int token_index = 0;
    int i = 0;
    while (tokens[token_index].type != PROGRAM_END)
    {
        Token current_token = tokens[token_index++];
        printf("\ntoken: ");
        dbg_token(current_token);

        if (current_token.type == OP)
        {
            printf("-- OP %c\n", current_token.value.op_type);
            program[i++] = current_token.value.op_type;
        }
        else if (current_token.type == NUMBER)
        {
            char num_str[64];
            sprintf(num_str, "%d!", current_token.value.n);
            int _i = 0;
            while (num_str[_i] != '!')
            {
                program[i++] = num_str[_i++];
            }
        }
        else if (current_token.type == OPEN_PAR)
        {
            program[i++] = '(';
        }
        else if (current_token.type == CLOSE_PAR)
        {
            program[i++] = ')';
        }
        else
        {
            // program[i++] = current_token.type;
            program[i++] = '?';
        }
    }
}

int main()
{
    Token *tokens = malloc(512 * sizeof(Token));
    tokenise("./expr.meth", tokens);
    dbg_tokens(tokens);

    printf("\n\n");

    string c_program = malloc(4096 * sizeof(char));
    compile_c_to_str(tokens, c_program);

    printf("compiled c program: \n\n");
    printf("%s", c_program);

    free(c_program);
    free(tokens);
    return 0;
}
