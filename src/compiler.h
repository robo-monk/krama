#ifndef KRAMA_CCOMPILER_H
#define KRAMA_CCOMPILER_H

#include "ast.h"
#include "arena.h"
#include "hashmap.h"

typedef struct {
    Arena* arena;
    hashmap_t* macros;
} CompilerContext;

typedef struct c_program_t {
    vector_t headers;
    vector_t impls;
} c_program_t;

void compile(program_t program, const char* file_out);
#endif
