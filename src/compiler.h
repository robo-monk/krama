#ifndef KRAMA_CCOMPILER_H
#define KRAMA_CCOMPILER_H

#include "ast.h"
#include "arena.h"

#define MAX_HEADER_COUNT 1024
#define MAX_IMPLEMENTATION_COUNT 1024

typedef struct {
    Arena* arena;
} CompilerContext;

typedef struct c_program_t {
    char* headers[MAX_HEADER_COUNT];
    char* impls[MAX_IMPLEMENTATION_COUNT];
    int header_count;
    int impl_count;
} c_program_t;

void compile(program_t program, const char* file_out);
#endif
