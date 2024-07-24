#include "ast.h"
#include "stdarg.h"
#include "tokeniser.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_HEADER_COUNT 1024
#define MAX_IMPLEMENTATION_COUNT 1024

typedef struct {
    void* data;
    size_t capacity;
    size_t offset;
} Arena;

Arena arena_new(size_t capacity) {
    return (Arena) {
        .data = malloc(capacity),
        .capacity = capacity,
        .offset = 0
    };
}

void* arena_alloc(Arena *arena, size_t size) {
    if (arena->offset + size > arena->capacity) {
        printf("\n arena out of memory, implement resizing or regions...");
        exit(1);
    }

    void* ptr = arena->data + arena->offset;
    arena->offset += size;
    return ptr;
}

void arena_destroy(Arena *arena) {
    free(arena->data);
    arena->data = NULL;
    arena->capacity = 0;
    arena->offset = 0;
}

typedef struct {
    Arena* arena;
} CompilerContext;

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
            return string_arena_format(ctx->arena,
                "%s%s",
                token_type_to_string(e->data.prefix.operand.type),
                right_expr
            );
        }
        case EXPRESSION_TYPE_INFIX: {
            char* left_expr = compile_expression(ctx, program, e->data.infix.left);
            char* right_expr = compile_expression(ctx, program, e->data.infix.right);
            return string_arena_format(ctx->arena,
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
            return string_arena_format(ctx->arena, "void %s()\n%s", e->data.func_decl.name, fn_body);
        }

        case EXPRESSION_TYPE_BLOCK: {
            char* block = "  ";
            for (int i = 0; i < e->data.block.statement_count; i++) {
                char* stmt = compile_statement(ctx, program, &e->data.block.statements[i]);
                block = string_arena_format(ctx->arena, "%s\n  %s", block, stmt);
            }

            return string_arena_format(ctx->arena, "{%s\n}", block);
        }
    }
}

char* compile_statement(CompilerContext *ctx, c_program_t *program, statement_t *s) {
    switch (s->type) {
        case STATEMENT_TYPE_LET: {
            char* exp = compile_expression(ctx, program, s->data.let.identifier.value);
            printf("\ntype let exp out:: %s\n", exp);
            return string_arena_format(ctx->arena, "int %s = %s;",
                s->data.let.identifier.name,
                exp
            );
        }
        case STATEMENT_TYPE_DEFER:
            printf("\nDefer statement not implemented!");
            return NULL;
        case STATEMENT_TYPE_EXPRESSION: {
            char* exp = compile_expression(ctx, program, &s->data.expression);
            printf("\nexp out:: %s\n", exp);
            return string_arena_format(
                    ctx->arena,
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

    arena_destroy(&arena);
}
