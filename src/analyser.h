#ifndef KRAMA_ANALYSER_H
#define KRAMA_ANALYSER_H

#include "ast.h"
#include "compiler.h"
#include "hashmap.h"
#include "parser.h"
#include "tokeniser.h"

typedef struct scope_t {
    hashmap_t *table;
    struct scope_t *upper;
} scope_t;

typedef struct {
    char* name;
    expression_t *expression;
} scope_entry_t;

typedef struct {
    char* message;
    token_t token;
} analyser_error_t;

#define ANALYSER_MAX_ERROR_COUNT 28
typedef struct {
    int index;
    token_t* tokens;
    program_t program;
    analyser_error_t* errors[ANALYSER_MAX_ERROR_COUNT];
    unsigned int error_idx;
    CompilerContext *ctx;
} analyser_t;

analyser_t analyser_new(CompilerContext *ctx);
void analyser_destroy(analyser_t *a);
analyser_t analyse_program(parser_t *parser, CompilerContext *ctx);
#endif
