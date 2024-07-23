#include "ast.h"
#include "stdarg.h"
#include "tokeniser.h"
#include <assert.h>

#define MAX_HEADER_COUNT 1024
#define MAX_IMPLEMENTATION_COUNT 1024
#define LONG_STR_CHARS 2048


#define STRING_CHARS 1024
typedef struct {
    char cstr[STRING_CHARS];
} String;

String stringf(const char *fmt, ...)
{
    String result;
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(result.cstr, sizeof(result.cstr), fmt, args);
    assert(n >= 0);
    assert(n + 1 <= sizeof(result.cstr));
    va_end(args);
    return result;
}

typedef struct c_program_t {
    char* headers[MAX_HEADER_COUNT];
    char* impls[MAX_IMPLEMENTATION_COUNT];
    int header_count;
    int impl_count;
} c_program_t;

c_program_t c_program_new() {
    return (c_program_t) {
        .headers = {},
        .impls = {},
        .header_count = 0,
        .impl_count = 0,
    };
}



// char* format_string(const char* format, ...) {
//     char buffer[LONG_STR_CHARS];
//     va_list args;

//     va_start(args, format);
//     vsnprintf(buffer, sizeof(buffer), format, args);
//     va_end(args);

//     // return buffer;
//     // printf("\nallocated str\n");
//     // return strdup(buffer);
// }



void sformat(String *s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(s->cstr, sizeof(s->cstr), format, args);
    va_end(args);
}


void compile_expression(String *expr, c_program_t *program, expression_t *e){

    if (e == NULL) {
        printf("\nWARNING: tried to compile NULL expression...\n");
        return;
    }

    switch (e->type) {
        case EXPRESSION_TYPE_PREFIX: {
            String right_expr = {0};
            compile_expression(&right_expr ,program, e->data.prefix.right);

            sformat(
                expr,
                "%s%s",
                token_type_to_string(e->data.prefix.operand.type),
                right_expr.cstr
            );

            printf("\n----\n");
            printf("type prefix, expr is: %s\n", expr->cstr);
            printf("type prefix, right expr is: %s", right_expr.cstr);
            printf("\n----\n");
            break;
        }
        case EXPRESSION_TYPE_INFIX: {
            String left_expr = {0};
            compile_expression(&left_expr ,program, e->data.infix.left);

            String right_expr = {0};
            compile_expression(&right_expr ,program, e->data.infix.right);

            printf("\n----\n");
            printf("type infix, left expr is: %s\n", left_expr.cstr);
            printf("type infix, right expr is: %s", right_expr.cstr);
            printf("\n----\n");

            sformat(
                expr,
                "%s %s %s",
                left_expr.cstr,
                token_type_to_string(e->data.prefix.operand.type),
                right_expr.cstr
            );
            break;
        }
        case EXPRESSION_TYPE_LITERAL:
            sformat(expr, "%ld", e->data.literal.data.i64);
            break;
        case EXPRESSION_TYPE_IDENTIFIER:
            sformat(expr, "%s", e->data.identifier.name);
            break;
        case EXPRESSION_TYPE_BLOCK:
            break;
    }
}

void compile_statement(String *stmt, c_program_t *program, statement_t *s) {
    switch (s->type) {
        case STATEMENT_TYPE_LET: {
            String exp = {0};
            compile_expression(&exp, program, s->data.let.identifier.value);
            printf("\ntype let exp out:: %s\n", exp.cstr);
            return sformat(stmt, "int %s = %s;",
                s->data.let.identifier.name,
                exp.cstr
            );
        }
        case STATEMENT_TYPE_DEFER:
            printf("\nDefer statement not implemented!");
            return;
            // return "\ndefer statement compilation not implemented\n";
        case STATEMENT_TYPE_EXPRESSION: {
            String exp = {0};
            compile_expression(&exp, program, &s->data.expression);
            printf("\nexp out:: %s\n", exp.cstr);
            return sformat(
                    stmt,
                    "%s;",
                    exp.cstr
                );
        }
    }
}

void compile(program_t program, const char* file_out) {
    c_program_t cprogram = c_program_new();
    for (int i = 0; i < program.statement_count; i++) {
        statement_t st = program.statements[i];
        String stmt = {0};
        compile_statement(&stmt, &cprogram, &st);
        cprogram.impls[cprogram.impl_count++] = strdup(stmt.cstr);
    }

    for (int hi = 0; hi < cprogram.header_count; hi++) {
        printf("%d\n%s\n", hi, cprogram.headers[hi]);
    }
    printf("\n");
    for (int ii = 0; ii < cprogram.impl_count; ii++) {
        printf("\n%s\n", cprogram.impls[ii]);
    }
}
