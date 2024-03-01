#include "stdio.h"
#include "stdlib.h"
#include "utils.h"
#include "tokeniser.h"

// produces program that when run
void compile_c_to_str(Token *tokens, string program)
{
    int token_index = 0;
    int i = 0;
    while (tokens[token_index].type != PROGRAM_END)
    {
        Token current_token = tokens[token_index++];

        if (current_token.type == OP)
        {
            // printf("-- OP %c\n", current_token.value.op_type);
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
            program[i++] = '?';
        }
    }
}

int perform_op(int lhs, int rhs, OpType op)
{
    switch (op)
    {
    case ADD:
        return lhs + rhs;
    case MIN:
        return lhs - rhs;
    case MUL:
        return lhs * rhs;
    case DIV:
        return lhs + rhs;
    default:
        printf("urecognised OpType");
    }
}

int _evaluate(Token *tokens, int *token_index)
{
    // int token_index = 0;
    int i = 0;
    int result = 0;
    OpType current_op = -1;
    while (tokens[*token_index].type != PROGRAM_END)
    {
        Token current_token = tokens[(*token_index)++];
        printf("\ntoken: ");
        dbg_token(current_token);

        if (current_token.type == OP && (current_token.value.op_type == MUL || current_token.value.op_type == DIV))
        {
            current_op = current_token.value.op_type;
        }
        else if (current_token.type == NUMBER)
        {
            if (current_op == -1)
            {
                result = current_token.value.n;
            }
            else
            {
                result = perform_op(result, current_token.value.n, current_op);
            }
        }
        else if (current_token.type == OPEN_PAR)
        {
            result = perform_op(result, _evaluate(tokens, token_index), current_op);
        }
        else if (current_token.type == CLOSE_PAR)
        {
            return result;
        }
    }
    return result;
}

int evaluate(Token *tokens)
{
    int token_index = 0;
    return _evaluate(tokens, &token_index);
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

    // int r = evaluate(tokens);
    // printf("result -> %d", r);

    free(c_program);
    free(tokens);
    return 0;
}
