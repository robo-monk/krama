#ifndef KRAMA_MACROS_H
#define KRAMA_MACROS_H

#include "analyser.h"
#include "arena.h"
#include "ast.h"
#include "compiler.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// typedef ptype_t (*macro_get_type_callback)(scope_t *scope, vector_t *params);
// typedef char* (*macro_compile_callback)(CompilerContext *ctx, c_program_t *program, expression_t *exp) {

typedef ptype_t (*macro_get_type_callback)(scope_t *scope, vector_t *params);
typedef char* (*macro_compile_callback)(CompilerContext *ctx, c_program_t *program, expression_t *exp);
static char* cast_compile_callback(CompilerContext *ctx, c_program_t *program, expression_t *exp);

macro_get_type_callback get_macro_type_callback(const char* identifier);
macro_compile_callback get_macro_compile_callback(const char* identifier);
#endif
