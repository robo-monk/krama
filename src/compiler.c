#include "compiler.h"
#include "arena.h"
#include "ast.h"
#include "stdarg.h"
#include "tokeniser.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

c_program_t c_program_new() {
    return (c_program_t) {
        .headers = {},
        .impls = {},
        .header_count = 0,
        .impl_count = 0,
    };
}

char* string_arena_format_overwrite(Arena *arena, const char* overwrite_ptr, const char* fmt, ...) {
    // printf("\noverwrite ptr: %s\n",overwrite_ptr);
    // printf("\nfmt: %s\n", fmt);
    assert(overwrite_ptr == arena->last_ptr);
    size_t last_bytes = ((arena->data+arena->offset) - arena->last_ptr);
    arena->offset -= last_bytes; // go back

va_list args;
    va_start(args, fmt);

    va_list copy;
    va_copy(copy, args);
    size_t length = vsnprintf(NULL, 0, fmt, copy) + 1;
    char* tempstr = malloc(length * sizeof(char));
    vsnprintf(tempstr, length, fmt, args);
    char* str = arena_alloc(arena, length * sizeof(char));
    strcpy(str, tempstr);
    va_end(args);
    free(tempstr);
    return str;
}

char* string_arena_format(Arena *arena, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list copy;
    va_copy(copy, args);
    size_t length = vsnprintf(NULL, 0, fmt, copy) + 1;
    char* str = arena_alloc(arena, length * sizeof(char));
    vsnprintf(str, length, fmt, args);
    va_end(args);
    return str;
}


char* compile_statement(CompilerContext *ctx, c_program_t *program, statement_t *s);

char* compile_expression(CompilerContext *ctx, c_program_t *program, expression_t *e){

    if (e == NULL) {
        printf("\nWARNING: tried to compile NULL expression...\n");
        return NULL;
    }

    switch (e->type) {
        case EXPRESSION_TYPE_PREFIX: {
            char *right_expr = compile_expression(ctx, program, e->data.prefix.right);
            return string_arena_format_overwrite(ctx->arena, right_expr,
                "%s%s",
                token_type_to_string(e->data.prefix.operand.type),
                right_expr
            );
        }
        case EXPRESSION_TYPE_INFIX: {
            char* left_expr = compile_expression(ctx, program, e->data.infix.left);
            char* right_expr = compile_expression(ctx, program, e->data.infix.right);
            return string_arena_format_overwrite(ctx->arena, right_expr,
                "%s %s %s",
                left_expr,
                token_type_to_string(e->data.prefix.operand.type),
                right_expr
            );
        }
        case EXPRESSION_TYPE_LITERAL:
            return string_arena_format(ctx->arena, "%ld", e->data.literal.data.i64);
        case EXPRESSION_TYPE_IDENTIFIER:
            return string_arena_format(ctx->arena, "%s", e->data.identifier.name);
        case EXPRESSION_TYPE_FUNC_DECL:{
            char* fn_body = compile_expression(ctx ,program, e->data.func_decl.value);
            return string_arena_format_overwrite(ctx->arena, fn_body, "void %s()\n%s", e->data.func_decl.name, fn_body);
        }
        case EXPRESSION_TYPE_BLOCK: {
            char* block = "  ";
            for (int i = 0; i < e->data.block.statement_count; i++) {
                char* stmt = compile_statement(ctx, program, &e->data.block.statements[i]);
                block = string_arena_format_overwrite(ctx->arena, stmt, "%s\n  %s", block, stmt);
            }

            return string_arena_format_overwrite(ctx->arena, block, "{%s\n}", block);
        }
        case EXPRESSION_TYPE_CALL: {
            char* args = NULL;
            for (int i = 0; i < e->data.call.arguments.count; i++) {
                char* expr = compile_expression(ctx, program, vector_get(&e->data.call.arguments, i));
                if (args == NULL) {
                    args = expr;
                } else {
                    args = string_arena_format_overwrite(ctx->arena, expr, "%s,%s", args, expr);
                }
            }
            return string_arena_format(ctx->arena, "%s(%s)", arena_strdup(ctx->arena, e->data.call.identifier_name), args);
        }
        case EXPRESSION_TYPE_RETURN: {
            char* ret_exp = compile_expression(ctx, program, e->data.return_exp.expression);
            if (ret_exp == NULL) return string_arena_format(ctx->arena, "return");
            return string_arena_format_overwrite(ctx->arena, ret_exp, "return %s", ret_exp);
        }
        case EXPRESSION_TYPE_CONDITIONAL:
        case EXPRESSION_TYPE_FOR:
            return string_arena_format(ctx->arena, "%s", "Not implemented");
            break;
        }
}

char* compile_statement(CompilerContext *ctx, c_program_t *program, statement_t *s) {
    switch (s->type) {
        case STATEMENT_TYPE_LET: {
            char* exp = compile_expression(ctx, program, s->data.let.identifier.value);
            return string_arena_format_overwrite(ctx->arena, exp, "int %s = %s;",
                s->data.let.identifier.name,
                exp
            );
        }
        case STATEMENT_TYPE_DEFER:
            return string_arena_format(ctx->arena, " [Not implemented defer stmt] ");
        case STATEMENT_TYPE_EXPRESSION: {
            char* exp = compile_expression(ctx, program, &s->data.expression);
            return string_arena_format_overwrite(
                    ctx->arena,
                    exp,
                    "%s;",
                    exp
                );
        }
    }
}

void compile(program_t program, const char* file_out) {
    Arena arena = arena_new(1024*1024);
    CompilerContext ctx = (CompilerContext) {
        .arena = &arena
    };

    c_program_t cprogram = c_program_new();
    for (int i = 0; i < program.statement_count; i++) {
        char* stmt = compile_statement(&ctx, &cprogram, &program.statements[i]);
        cprogram.impls[cprogram.impl_count++] = stmt;
    }

    printf("\n---- %s.c ---- \n", file_out);
    for (int hi = 0; hi < cprogram.header_count; hi++) {
        printf("%d\n%s\n", hi, cprogram.headers[hi]);
    }

    printf("\n");
    for (int ii = 0; ii < cprogram.impl_count; ii++) {
        printf("\n%s\n", cprogram.impls[ii]);
    }

    printf("\n---\n\n");
    printf("[Compiler Stats] Compiler Arena contained %ld bytes out of total %ld bytes (%ld%%)\n", arena.offset, arena.capacity, 100*arena.offset/arena.capacity);
    arena_destroy(&arena);
}
