#ifndef KRAMA_CCOMPILER_H
#define KRAMA_CCOMPILER_H

#include "ast.h"
#include "arena.h"
#include "hashmap.h"

typedef struct CompilerContext CompilerContext;
typedef char* (*fn_name_mangler)(CompilerContext *ctx, expression_t *exp);

struct CompilerContext {
    Arena* arena;
    hashmap_t* macros;
    hashmap_t* types;
    fn_name_mangler fn_mangle;
    hashmap_t* fn_declerations;
};

typedef struct c_program_t {
    vector_t headers;
    vector_t impls;
} c_program_t;

char* fn_expr_name_mangle(CompilerContext *ctx, expression_t *exp);
char* fn_generic_name_mangle(CompilerContext *ctx, expression_t *exp);
void compile(program_t *program, CompilerContext *ctx, const char* file_out);
// void compile(program_t program, const char* file_out);
#endif
