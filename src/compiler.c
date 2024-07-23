#include "ast.h"
#include "stdarg.h"
#include "tokeniser.h"
#include <assert.h>

#define MAX_HEADER_COUNT 1024
#define MAX_IMPLEMENTATION_COUNT 1024

#define STRING_CHARS 1024
typedef struct {
    char cstr[STRING_CHARS];
    char* dstr;
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

void sformat(String *s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(s->cstr, sizeof(s->cstr), format, args);
    va_end(args);
}



unsigned long slen(String *s) {
    if (s->dstr != NULL) {
        // string was dynamically allocated, and has to be freed.
        return strlen(s->dstr);
    }

    return sizeof(s->cstr);
}

void sconsume(String *s, char* flush) {
    if (s->dstr != NULL) {
        // string was dynamically allocated, and has to be freed.
        // move string to stack and free addr
        int len = strlen(s->dstr);
        printf("str len is %d\n", len);
        for (int i = 0; i<len; i++) {
            flush[i] = s->dstr[i];
        }
        free(s->dstr);
        return;
    }

    for (int i = 0; i<sizeof(s->cstr); i++) {
        flush[i] = s->cstr[i];
    }
    // flush = &*s->cstr;
}


void compile_statement(String *stmt, c_program_t *program, statement_t *s);

void compile_expression(String *expr, c_program_t *program, expression_t *e){

    if (e == NULL) {
        printf("\nWARNING: tried to compile NULL expression...\n");
        return;
    }

    switch (e->type) {
        case EXPRESSION_TYPE_PREFIX: {
            String right_expr = {0};
            compile_expression(&right_expr ,program, e->data.prefix.right);

            char str[slen(&right_expr)];
            sconsume(&right_expr, str);

            return sformat(
                expr,
                "%s%s",
                token_type_to_string(e->data.prefix.operand.type),
                str
            );
        }
        case EXPRESSION_TYPE_INFIX: {
            String left_expr = {0};
            compile_expression(&left_expr ,program, e->data.infix.left);

            String right_expr = {0};
            compile_expression(&right_expr ,program, e->data.infix.right);


            char lstr[slen(&left_expr)];
            sconsume(&left_expr, lstr);

            char rstr[slen(&right_expr)];
            sconsume(&right_expr, rstr);

            return sformat(
                expr,
                "%s %s %s",
                // left_expr.cstr,
                lstr,
                token_type_to_string(e->data.prefix.operand.type),
                rstr
            );
        }

        case EXPRESSION_TYPE_LITERAL:
            return sformat(expr, "%ld", e->data.literal.data.i64);
        case EXPRESSION_TYPE_IDENTIFIER:
            return sformat(expr, "%s", e->data.identifier.name);
        case EXPRESSION_TYPE_BLOCK: {
            String block = {0};
            for (int i = 0; i < e->data.block.statement_count; i++) {
                String newst = {0};
                compile_statement(&newst, program, &e->data.block.statements[i]);
                char newst_stack[slen(&newst)];
                sconsume(&newst, newst_stack);

                char curr[slen(&block)];
                sconsume(&block, curr);
                sformat(&block, "%s\n  %s", curr, newst_stack);
            }

            char curr[slen(&block)];
            sconsume(&block, curr);
            return sformat(expr, "{%s\n}", curr);
        }
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
