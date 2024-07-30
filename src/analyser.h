#ifndef KRAMA_ANALYSER_H
#define KRAMA_ANALYSER_H
#include "ast.h"

typedef struct {
    char* message;
    token_t token;
} analyser_error_t;

typedef struct {
    Arena arena;
} AnalyserContext;

#define ANALYSER_MAX_ERROR_COUNT 28
typedef struct {
    int index;
    token_t* tokens;
    program_t program;
    analyser_error_t* errors[ANALYSER_MAX_ERROR_COUNT];
    unsigned int error_idx;
    AnalyserContext ctx;
} analyser_t;

analyser_t analyser_new();
void analyser_destroy(analyser_t *a);
void analyse_program(program_t *program);
#endif
