#include "compiler.h"
#include "arena.h"
#include "ast.h"
#include "hashmap.h"
// #include "macros.h"
#include "parser.h"
#include "stdarg.h"
#include "tokeniser.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>


c_program_t c_program_new() {
    return (c_program_t) {
        .headers = vector_new(16, sizeof(char*)),
        .impls = vector_new(16, sizeof(char*)),
    };
}


char* compile_type_internal(CompilerContext *ctx, type_t *type, char deref_symbol) {
    assert(type != NULL);
    if (type->kind == TYPE_KIND_UNKNOWN) return string_arena_format(ctx->arena, "(unknown)");
    if (type->kind == TYPE_KIND_PRIMITIVE) return string_arena_format(ctx->arena, "%s", type->info.primitive);
    assert(type->kind == TYPE_KIND_POINTER);
    char* upper = compile_type_internal(ctx, type->info.pointer, deref_symbol);
    return string_arena_format_overwrite(ctx->arena, upper, "%s%c", upper, deref_symbol);
}

char* compile_type(CompilerContext *ctx, type_t *type) {
    return compile_type_internal(ctx, type, '*');
}


char* fn_expr_name_mangle(CompilerContext *ctx, expression_t *exp) {
    if (exp->type == EXPRESSION_TYPE_FUNC_DECL) {
        char* fn_name = arena_strdup(ctx->arena, exp->data.func_decl.name);
        for (int i = 0; i<exp->data.func_decl.params.count; i++) {
            identifier_expression_t *id = (identifier_expression_t*) vector_get_ptr(&exp->data.func_decl.params, i);
            assert(id->type.kind != TYPE_KIND_UNKNOWN);
            char* type = compile_type_internal(ctx, &id->type, '_');
            fn_name = string_arena_format_overwrite(ctx->arena, type, "%s_%s", fn_name, type);
        }
        return fn_name;
    } else if (exp->type == EXPRESSION_TYPE_CALL) {
        char* fn_name = arena_strdup(ctx->arena, exp->data.call.identifier_name);
        for (int i = 0; i<exp->data.call.arguments.count; i++) {
            expression_t *arg = (expression_t*) vector_get_ptr(&exp->data.call.arguments, i);
            assert(exp && exp->resultType.kind != TYPE_KIND_UNKNOWN);
            assert(arg != NULL);
            char* type = compile_type_internal(ctx, &arg->resultType, '_');
            fn_name = string_arena_format_overwrite(ctx->arena, type, "%s_%s", fn_name, type);
        }
        return fn_name;
    }
    assert(0);
}

char* compile_statement(CompilerContext *ctx, c_program_t *program, statement_t *s);
char* compile_expression(CompilerContext *ctx, c_program_t *program, expression_t *e);

typedef char* (*macro_compile_callback)(CompilerContext *ctx, c_program_t *program, vector_t *arguments);

void register_macro_handler(CompilerContext *ctx, char* id, macro_compile_callback cb) {
    hashmap_insert(ctx->macros, id, cb);
}

static char* raw_c_compile_callback(CompilerContext *ctx, c_program_t *program, vector_t *args) {
    assert(args->count == 1);
    expression_t *exp = (expression_t*) vector_get_ptr(args, 0);
    assert(exp->type == EXPRESSION_TYPE_LITERAL);
    assert(exp->data.literal.kind == LITERAL_KIND_STRING);
    assert(exp->data.literal.data.string != NULL);
    return string_arena_format(ctx->arena, "%s", exp->data.literal.data.string);

}

static char* malloc_compile_callback(CompilerContext *ctx, c_program_t *program, vector_t *args) {
    assert(args->count == 1);
    expression_t *exp = (expression_t*) vector_get_ptr(args, 0);
    // assert(exp->resultType == PTYPE_I64);
    char* size_expr = compile_expression(ctx, program, exp);
    return string_arena_format_overwrite(ctx->arena, size_expr, "malloc(%s)", size_expr);

}

static char* set_compile_callback(CompilerContext *ctx, c_program_t *program, vector_t *args) {
    assert(args->count == 2);
    expression_t *exp = (expression_t*) vector_get_ptr(args, 0);
    // assert(exp->type == EXPRESSION_TYPE_IDENTIFIER);
    // has to be pointer
    // assert(exp->data.identifier.type == PTYPE_I64);
    char* ptr_expression = compile_expression(ctx, program, exp);

    expression_t *val_exp = (expression_t*) vector_get_ptr(args, 1);
    printf("\n ---> value is \n");
    debug_expression(val_exp, 0);
    printf("\n ---> value is \n");
    char* val = compile_expression(ctx, program, val_exp);
    return string_arena_format_overwrite(ctx->arena, val, "*(%s) = %s", ptr_expression, val);
}

static char* get_compile_callback(CompilerContext *ctx, c_program_t *program, vector_t *args) {
    assert(args->count == 1);
    expression_t *exp = (expression_t*) vector_get_ptr(args, 0);
    char* ptr_expression = compile_expression(ctx, program, exp);
    assert(ptr_expression != NULL);
    return string_arena_format_overwrite(ctx->arena, ptr_expression, "*(%s)", ptr_expression);
}

static char* free_compile_callback(CompilerContext *ctx, c_program_t *program, vector_t *args) {
    assert(args->count == 1);
    expression_t *exp = (expression_t*) vector_get_ptr(args, 0);
    // assert(exp->resultType == PTYPE_I64);
    char* size_expr = compile_expression(ctx, program, exp);
    return string_arena_format_overwrite(ctx->arena, size_expr, "free(%s)", size_expr);
}

char* compile_static_call(CompilerContext *ctx, c_program_t *program, expression_t *exp) {
    macro_compile_callback cb = hashmap_get(ctx->macros, exp->data.call.identifier_name);
    assert(cb != NULL);
    return cb(ctx, program, &exp->data.call.arguments);
}


char* compile_comma_seperated_exprs(CompilerContext *ctx, c_program_t *program, vector_t *args_vector) {
    char* args = NULL;
    for (int i = 0; i < args_vector->count; i++) {
        char* expr = compile_expression(ctx, program, (expression_t*) vector_get_ptr(args_vector, i));
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
        identifier_expression_t* expr = (identifier_expression_t*) vector_get_ptr(params, i);
        char* type = compile_type(ctx, &expr->type);
        if (args == NULL) {
            // args = string_arena_format(ctx->arena, "%s %s", ptype_to_ctype(expr->type), expr->name);
            args = string_arena_format_overwrite(ctx->arena, type, "%s %s", type, expr->name);
        } else {
            args = string_arena_format_overwrite(ctx->arena, type, "%s, %s %s", args, type, expr->name);
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
            if (e->data.prefix.operand.type == TOKEN_AS) {
                char* compiled_type = compile_type(ctx, &e->data.infix.left->resultType);
                char* left_expr = compile_expression(ctx, program, e->data.infix.left);

                assert(compiled_type != NULL);
                assert(left_expr != NULL);
                return string_arena_format_overwrite(ctx->arena, left_expr,
                                "((%s) %s)",
                                compiled_type,
                                left_expr
                            );
            }

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
            switch (e->data.literal.kind) {
                case LITERAL_KIND_I64:
                    return string_arena_format(ctx->arena, "%ld", e->data.literal.data.i64);
                case LITERAL_KIND_F64:
                    return string_arena_format(ctx->arena, "%lf", e->data.literal.data.f64);
                case LITERAL_KIND_CHARACTER:
                    return string_arena_format(ctx->arena, "'%c'", e->data.literal.data.character);
                case LITERAL_KIND_STRING: {

                    return string_arena_format(ctx->arena, "\"%s\"", e->data.literal.data.string);
                    // return  e->data.literal.data.string;
                    // int len = strlen(e->data.literal.data.string);
                    // assert(len >= 2);
                    // char* str = string_arena_format(ctx->arena, "{");
                    // for (int i = 1; i < len-1; i++) {
                    //     str = string_arena_format_overwrite(ctx->arena, str, "%s'%c',", str, e->data.literal.data.string[i]);
                    // }
                    // str = string_arena_format_overwrite(ctx->arena, str, "%s}", str);
                    // printf("\n->%s\n",str);
                    // // assert(0);
                    // // return string_arena_format(ctx->arena, "%s", e->data.literal.data.string);
                    // return str;
                }
                case LITERAL_KIND_TYPE: {
                    assert(0);
                }
            }
        }
        case EXPRESSION_TYPE_IDENTIFIER:
            return string_arena_format(ctx->arena, "%s", e->data.identifier.name);
        case EXPRESSION_TYPE_IDENTIFIER_ASSIGNMENT: {
            char* val = compile_expression(ctx, program, e->data.identifier.value);
            return string_arena_format_overwrite(ctx->arena, val, "%s = %s", e->data.identifier.name, val);
        }
        case EXPRESSION_TYPE_EXTERN_FUNC_DECL:
        case EXPRESSION_TYPE_FUNC_DECL:{
            char* return_type = compile_type(ctx, &e->data.func_decl.type);
            char *params = compile_comma_seperated_params(ctx, program, &e->data.func_decl.params);
            if (e->data.func_decl.value != NULL) {
                char* fn_body = compile_expression(ctx ,program, e->data.func_decl.value);
                return string_arena_format_overwrite(ctx->arena, fn_body, "%s %s(%s)\n%s",
                    return_type,
                    e->data.func_decl.name,
                    params,
                    fn_body);
            } else {
                // return "\0";
                return string_arena_format(ctx->arena, "// external %s %s(%s)",
                    return_type,
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
        case EXPRESSION_TYPE_STATIC_CALL: {
            return compile_static_call(ctx, program, e);
        }
        case EXPRESSION_TYPE_CALL: {
            char* args = compile_comma_seperated_exprs(ctx, program, &e->data.call.arguments);
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
            char* ctype = s->data.let.identifier.type.kind == TYPE_KIND_UNKNOWN ?
                compile_type(ctx, &s->data.expression.resultType)
                : compile_type(ctx, &s->data.let.identifier.type);

            char* exp = compile_expression(ctx, program, s->data.let.identifier.value);

            char* ssk = string_arena_format_overwrite(ctx->arena, exp, "%s %s = %s;",
                ctype,
                s->data.let.identifier.name,
                exp
            );

            return ssk;
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

void compile(program_t *program, CompilerContext *ctx, const char* file_out) {

    register_macro_handler(ctx, "@malloc", malloc_compile_callback);
    register_macro_handler(ctx, "@free", free_compile_callback);
    register_macro_handler(ctx, "@set", set_compile_callback);
    register_macro_handler(ctx, "@get", get_compile_callback);
    register_macro_handler(ctx, "@raw_c", raw_c_compile_callback);

    c_program_t cprogram = c_program_new();
    vector_push_ptr(&cprogram.headers, string_arena_format(ctx->arena, "#include <stdio.h>"));
    vector_push_ptr(&cprogram.headers, string_arena_format(ctx->arena, "#include <stdlib.h>"));
    vector_push_ptr(&cprogram.headers, string_arena_format(ctx->arena, "#include <string.h>"));
    vector_push_ptr(&cprogram.headers, string_arena_format(ctx->arena, "#include <stdbool.h>"));

    for (int i = 0; i < program->statements.count; i++) {
        // statement_t **s = (statement_t**) vector_get(&program.statements, i);
        char* stmt = compile_statement(ctx, &cprogram, (statement_t*) vector_get_ptr(&program->statements, i));
        vector_push_ptr(&cprogram.impls, stmt);
    }


    printf("\n---- %s ---- \n", file_out);
    FILE *file_ptr = fopen(file_out, "w");

    if (file_ptr == NULL) {
        printf("\n Error creating output `%s` file\n", file_out);
        exit(1);
    }

    for (int i = 0; i < cprogram.headers.count; i++) {
        fprintf(file_ptr, "%s\n", (char*) vector_get_ptr(&cprogram.headers, i));
    }

    fprintf(file_ptr, "\n");
    for (int ii = 0; ii < cprogram.impls.count; ii++) {
        fprintf(file_ptr, "\n%s\n", (char*) vector_get_ptr(&cprogram.impls, ii));
    }

    printf("\n---\n\n");

    vector_free(&cprogram.impls);
    vector_free(&cprogram.headers);
    // arena_destroy(&arena);
}
