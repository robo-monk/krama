#include "statements.h"

Statement new_stmt(StatementType type, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value)
{
    Statement s = {
        .type = type,
        .left = left,
        .right = right,
        .start = start,
        .end = end,
        .raw_value = raw_value};
    return s;
}
Statement new_bin_expr_stmt(OpType op, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value)
{
    Statement s = new_stmt(BinOp, left, right, start, end, raw_value);
    s.bin_op.op = op;
    return s;
}

Statement new_i32_literal_stmt(int value, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value)
{
    Statement s = new_stmt(Literal, left, right, start, end, raw_value);
    s.literal.type = LiteralType_i32;
    s.literal.i32_value = value;

    return s;
}

Statement new_f64_literal_stmt(double value, Statement *left, Statement *right, unsigned int start, unsigned int end, string raw_value)
{
    Statement s = new_stmt(Literal, left, right, start, end, raw_value);
    s.literal.type = LiteralType_f64;
    s.literal.f64_value = value;
    return s;
}
