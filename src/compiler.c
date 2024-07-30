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

char* ptype_to_ctype(ptype_t t) {
    switch (t) {
    case PTYPE_I64:
        return "long";
    case PTYPE_I32:
        return "int";
    case PTYPE_I16:
        return "short";
    case PTYPE_U64:
        return "unsigned long";
    case PTYPE_U32:
        return "unsigned int";
    case PTYPE_U16:
        return "unsigned short";
    case PTYPE_U8:
        return "char";
    case PTYPE_F64:
        return "double";
    case PTYPE_F32:
        return "float";
    case PTYPE_F16:
        return "float";
    case PTYPE_CHAR:
        return "char";
    case PTYPE_VOID:
        return "void";
    case PTYPE_UNKNOWN:
        return "[UNKNOWN]";
    case PTYPE_ANY:
        return "void*";
    case PTYPE_BOOL:
        return "int";
    }
}

char* string_arena_format_overwrite(Arena *arena, const char* overwrite_ptr, const char* fmt, ...) {
    // printf("\noverwrite ptr: [%s]\n",overwrite_ptr);
    // printf("\nfmt: [%s]\n", fmt);
    assert(overwrite_ptr != NULL);
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
char* compile_expression(CompilerContext *ctx, c_program_t *program, expression_t *e);

char* compile_comma_seperated_exprs(CompilerContext *ctx, c_program_t *program, vector_t *args_vector) {
    char* args = NULL;
    for (int i = 0; i < args_vector->count; i++) {
        char* expr = compile_expression(ctx, program, vector_get(args_vector, i));
        expression_t* e = vector_get(args_vector, i);
        printf("--> |EXPR| %d\n", e->type);
        if (args == NULL) {
            args = expr;
        } else {
            args = string_arena_format_overwrite(ctx->arena, expr, "%s,%s", args, expr);
        }
    }
    if (args == NULL) return "";
    return args;
}

char* compile_comma_seperated_params(CompilerContext *ctx, c_program_t *program, vector_t *params) {
    char* args = NULL;
    for (int i = 0; i < params->count; i++) {
        identifier_expression_t* expr = vector_get(params, i);
        if (args == NULL) {
            args = string_arena_format(ctx->arena, "%s %s", ptype_to_ctype(expr->type), expr->name);
        } else {
            args = string_arena_format_overwrite(ctx->arena, args, "%s, %s %s", args, ptype_to_ctype(expr->type), expr->name);
        }
    }

    if (args == NULL) return string_arena_format(ctx->arena, "void");
    return args;
}

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
        case EXPRESSION_TYPE_LITERAL: {
            switch (e->data.literal.type) {
                case LITERAL_TYPE_I64:
                    return string_arena_format(ctx->arena, "%ld", e->data.literal.data.i64);
                case LITERAL_TYPE_F64:
                    return string_arena_format(ctx->arena, "%lf", e->data.literal.data.f64);
                case LITERAL_TYPE_CHARACTER:
                    return string_arena_format(ctx->arena, "%c", e->data.literal.data.character);
                case LITERAL_TYPE_STRING:
                    // return  e->data.literal.data.string;
                    return string_arena_format(ctx->arena, "%s", e->data.literal.data.string);
            }
        }
        case EXPRESSION_TYPE_IDENTIFIER:
            return string_arena_format(ctx->arena, "%s", e->data.identifier.name);
        case EXPRESSION_TYPE_FUNC_DECL:{
            char *params = compile_comma_seperated_params(ctx, program, &e->data.func_decl.params);
            if (e->data.func_decl.value != NULL) {
                char* fn_body = compile_expression(ctx ,program, e->data.func_decl.value);
                return string_arena_format_overwrite(ctx->arena, fn_body, "%s %s(%s)\n%s",
                    ptype_to_ctype(e->data.func_decl.type),
                    e->data.func_decl.name,
                    params,
                    fn_body);
            } else {
                // return "\0";
                return string_arena_format(ctx->arena, "// external %s %s(%s)",
                    ptype_to_ctype(e->data.func_decl.type),
                    e->data.func_decl.name,
                    params);
            }
        }
        case EXPRESSION_TYPE_BLOCK: {
            char* block = NULL;
            for (int i = 0; i < e->data.block.statement_count; i++) {
                char* stmt = compile_statement(ctx, program, &e->data.block.statements[i]);
                if (block == NULL) {
                    block = string_arena_format(ctx->arena, "  %s", stmt);
                } else {
                    block = string_arena_format_overwrite(ctx->arena, stmt, "%s\n  %s", block, stmt);
                }
            }

            if (block == NULL) {
                return string_arena_format(ctx->arena, "{}");
            }
            return string_arena_format_overwrite(ctx->arena, block, "{\n%s\n}", block);
        }
        case EXPRESSION_TYPE_CALL: {
            char* args = compile_comma_seperated_exprs(ctx, program, &e->data.call.arguments);
            printf("\nARGS ARE %s\n", args);
            return string_arena_format(ctx->arena, "%s(%s)", arena_strdup(ctx->arena, e->data.call.identifier_name), args);
        }
        case EXPRESSION_TYPE_RETURN: {
            char* ret_exp = compile_expression(ctx, program, e->data.return_exp.expression);
            if (ret_exp == NULL) return string_arena_format(ctx->arena, "return");
            return string_arena_format_overwrite(ctx->arena, ret_exp, "return %s", ret_exp);
        }
        case EXPRESSION_TYPE_CONDITIONAL: {
            char* predicate = compile_expression(ctx, program, e->data.conditional.predicate);
            char* success_branch = compile_expression(ctx, program, e->data.conditional.success_branch);
            if (success_branch == NULL) {
                printf("no success branch\n");
            }
            char* stmt = string_arena_format_overwrite(ctx->arena, success_branch, "if (%s) %s", predicate, success_branch);
            if (e->data.conditional.fail_branch != NULL) {
                char* fail_branch = compile_expression(ctx, program, e->data.conditional.success_branch);
                stmt = string_arena_format_overwrite(ctx->arena, fail_branch, "%s else %s", stmt, fail_branch);
            }
            return stmt;
        }
        case EXPRESSION_TYPE_FOR:
            return string_arena_format(ctx->arena, "%s", "Not implemented");
            break;
        }
}

char* compile_statement(CompilerContext *ctx, c_program_t *program, statement_t *s) {
    switch (s->type) {
        case STATEMENT_TYPE_LET: {
            char* exp = compile_expression(ctx, program, s->data.let.identifier.value);
            char* ctype = ptype_to_ctype(s->data.let.identifier.type);
            if (s->data.let.identifier.type == PTYPE_UNKNOWN) {
                ctype= ptype_to_ctype(s->data.expression.resultType);
            }
            return string_arena_format_overwrite(ctx->arena, exp, "%s %s = %s;",
                // ptype_to_ctype(s->data.let.identifier.type),
                ctype,
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

    cprogram.headers[cprogram.header_count++] = "#include <stdio.h>";

    printf("\n---- %s ---- \n", file_out);

    FILE *file_ptr = fopen(file_out, "w");
    if (file_ptr == NULL) {
        printf("\n Error creating output `%s` file", file_out);
        exit(1);
    }

    for (int i = 0; i < cprogram.header_count; i++) {
        fprintf(file_ptr, "%s\n", cprogram.headers[i]);
    }

    fprintf(file_ptr, "\n");
    for (int ii = 0; ii < cprogram.impl_count; ii++) {
        fprintf(file_ptr, "\n%s\n", cprogram.impls[ii]);
    }

    printf("\n---\n\n");
    printf("[Compiler Stats] Compiler Arena contained %ld bytes out of total %ld bytes (%ld%%)\n", arena.offset, arena.capacity, 100*arena.offset/arena.capacity);
    arena_destroy(&arena);
}
