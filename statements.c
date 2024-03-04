#include "stdlib.h"
#include "stdio.h"
#include "statements.h"

Statement *new_stmt(StatementType type, Statement *left, Statement *right, Token token)
{
    Statement *s = malloc(sizeof(Statement));
    s->type = type;
    s->left = left;
    s->right = right;
    s->token = token;
    return s;
}

Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right, Token token)
{
    Statement *s = new_stmt(BIN_OP, left, right, token);
    s->bin_op.op = op;
    return s;
}

Statement *new_i32_literal_stmt(int value, Token token)
{
    Statement *s = new_stmt(LITERAL, NULL, NULL, token);
    s->literal.type = LiteralType_i32;
    s->literal.i32_value = value;
    return s;
}

Statement *new_f64_literal_stmt(double value, Token token)
{
    Statement *s = new_stmt(LITERAL, NULL, NULL, token);
    s->literal.type = LiteralType_f64;
    s->literal.f64_value = value;
    return s;
}

void _alloc_statement_array(Program *program)
{
    if (program->len < program->max_len)
        return;
    program->statements = malloc(sizeof(Statement) * program->max_len);
}

Program *new_program()
{
    Program *program = malloc(sizeof(Program));
    program->idx = 0;
    program->len = 0;
    program->max_len = 0;
    return program;
}

void push_stmt(Statement *stmt, Program *program)
{
    program->len += 1;
    // optionally alloc more memory
    _alloc_statement_array(program);
    program->statements[program->len - 1] = stmt;
}

void throw_runtime_error(string msg)
{
    printf("%s", msg);
    exit(1);
}

int perform_i32_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op)
{
    switch (op)
    {
    case ADD:
        return lhs.i32_value + rhs.i32_value;
    case MIN:
        return lhs.i32_value - rhs.i32_value;
    case MUL:
        return lhs.i32_value * rhs.i32_value;
    case DIV:
        return lhs.i32_value / rhs.i32_value;
    default:
        throw_runtime_error("unsupported operation for type");
    }
}

double perform_f64_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op)
{
    switch (op)
    {
    case ADD:
        return lhs.f64_value + rhs.f64_value;
    case MIN:
        return lhs.f64_value - rhs.f64_value;
    case MUL:
        return lhs.f64_value * rhs.f64_value;
    case DIV:
        return lhs.f64_value / rhs.f64_value;
    default:
        throw_runtime_error("unsupported operation for type");
    }
}

ReturnValue perform_bin_op(ReturnValue lhs, ReturnValue rhs, OpType op)
{
    if (lhs.type != rhs.type)
        throw_runtime_error("Unmatched types!");

    ReturnValue v = {
        .type = lhs.type};

    switch (lhs.type)
    {
    case LiteralType_i32:
        v.i32_value = perform_i32_bin_op(lhs, rhs, op);
        break;
    case LiteralType_f64:
        v.f64_value = perform_f64_bin_op(lhs, rhs, op);
        break;
    default:
        throw_runtime_error("unsupported operation for type");
    }

    return v;
}

ReturnValue evaluate_statement(Statement *stmt)
{
    switch (stmt->type)
    {
    case LITERAL:
        return stmt->literal;
    case BIN_OP:
        return perform_bin_op(evaluate_statement(stmt->left), evaluate_statement(stmt->right), stmt->token.value.op_type);
    }

    throw_runtime_error("unsupported token");
}

ReturnValue exec_program(Program *program)
{
    ReturnValue result = {};

    for (int i = 0; i < program->len; i++)
    {
        result = evaluate_statement(program->statements[i]);
    }

    return result;
}
