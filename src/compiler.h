#ifndef KRAMA_CCOMPILER_H
#define KRAMA_CCOMPILER_H

#include "ast.h"
#include "arena.h"
#include "hashmap.h"

typedef char* (*fn_name_mangler)(char* fn_name, vector_t *arg_types);
typedef struct {
    Arena* arena;
    hashmap_t* macros;
    hashmap_t* types;
    fn_name_mangler fn_mangle;
} CompilerContext;

typedef struct c_program_t {
    vector_t headers;
    vector_t impls;
} c_program_t;

void compile(program_t *program, CompilerContext *ctx, const char* file_out);
// void compile(program_t program, const char* file_out);
#endif
