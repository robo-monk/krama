#include "macros.h"
#include "compiler.h"

const char* builtin_macro_symbols[] = {
    "@cast",
    "@cmalloc",
    "@cfree"
};

static ptype_t cast_type_callback(scope_t *scope, vector_t *params) {
    assert(params->count == 2);
    expression_t* type_expression = (expression_t*) vector_get(params, 1);
    assert(type_expression->type == EXPRESSION_TYPE_IDENTIFIER);
    return str_to_primitive_type(type_expression->data.identifier.name);
}

static ptype_t malloc_type_callback(scope_t *scope, vector_t *params) {
    return PTYPE_ANY;
}

static ptype_t free_type_callback(scope_t *scope, vector_t *params) {
    return PTYPE_VOID;
}


static char* malloc_compile_callback(CompilerContext *ctx, c_program_t *program, expression_t *exp) {
    return "";
}

static char* free_compile_callback(CompilerContext *ctx, c_program_t *program, expression_t *exp) {
    return "";
}


const macro_get_type_callback builtin_macro_type_callbacks[] = {
    cast_type_callback,
    malloc_type_callback,
    free_type_callback,
};


const macro_compile_callback builtin_macro_compile_callbacks[] = {
    cast_compile_callback,
    malloc_compile_callback,
    free_compile_callback,
};


#define ARRAY_SIZE(arr) (sizeof(arr))/(sizeof(arr[0]))
static_assert(ARRAY_SIZE(builtin_macro_symbols) == ARRAY_SIZE(builtin_macro_type_callbacks),
    "macro symbols and type callbacks length should match");

static_assert(ARRAY_SIZE(builtin_macro_symbols) == ARRAY_SIZE(builtin_macro_compile_callbacks),
    "macro symbols and compile callbacks length should match");


macro_get_type_callback get_macro_type_callback(const char* identifier) {
    for (int i = 0; i < ARRAY_SIZE(builtin_macro_symbols); i++) {
        if (strcmp(builtin_macro_symbols[i], identifier) == 0) {
            return builtin_macro_type_callbacks[i];
        }
    }
    return NULL;
}

macro_compile_callback get_macro_compile_callback(const char* identifier) {
    for (int i = 0; i < ARRAY_SIZE(builtin_macro_symbols); i++) {
        if (strcmp(builtin_macro_symbols[i], identifier) == 0) {
            return builtin_macro_compile_callbacks[i];
        }
    }
    return NULL;
}
