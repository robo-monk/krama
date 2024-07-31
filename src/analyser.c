#include "arena.h"
#include "analyser.h"
#include "ast.h"
#include "hashmap.h"
#include "parser.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// typedef struct scope_t {
//     hashmap_t *table;
//     struct scope_t *upper;
// } scope_t;

ptype_t get_ptype_from_static_call(scope_t *scope, expression_t *exp) {
    assert(strcmp(exp->data.call.identifier_name, "@cast") ==0 );
    assert(exp->data.call.arguments.count == 2);
    expression_t *type_expression = vector_get(&exp->data.call.arguments, 1);
    assert(type_expression->type == EXPRESSION_TYPE_IDENTIFIER);
    return str_to_primitive_type(type_expression->data.identifier.name);
}

void scope_define_entry(scope_t *scope, char* name, expression_t* exp) {
    hashmap_insert(scope->table, name, exp);
}

scope_t scope_create_sub(Arena *arena, scope_t *upper) {
    return (scope_t) {
        .upper = upper,
        .table = hashmap_create(NULL)
    };
}

expression_t* scope_get_entry(scope_t *scope, char* key) {
    while (scope != NULL && scope->table != NULL) {
        expression_t *entry = hashmap_get(scope->table, key);
        if (entry != NULL) return entry;
        scope = scope->upper;
    };
    // printf("\n [could not find variable `%s` ]\n", key);
    return NULL;
}

void analyser_error_print(analyser_error_t *error) {
    fprintf(stderr, "\nANALYSER ERROR: %s\n", error->message);
}

void analyser_error_create(analyser_t *analyser, const char *format, ...) {
    if (analyser->error_idx >= ANALYSER_MAX_ERROR_COUNT) {

        for (int i = 0; i < analyser->error_idx; i++) {
            analyser_error_print(analyser->errors[i]);
        }

        fprintf(stderr, "\nToo many analysis errors.\n");
        exit(1);
    }
    va_list args;
    va_start(args, format);

    analyser_error_t *error = arena_alloc(&analyser->ctx.arena, sizeof(analyser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = arena_alloc(&analyser->ctx.arena, msg_len);

    // Format the message
    vsnprintf(error->message, msg_len, format, args);
    va_end(args);

    // error->token = token;
    analyser->errors[analyser->error_idx++] = error;

    printf("\n(!) ANALYSER ERROR: %s\n", error->message);
}

bool analyser_assert(bool predicate, analyser_t *analyser, const char *format, ...) {
    if (predicate == true) return true;

    if (analyser->error_idx >= ANALYSER_MAX_ERROR_COUNT) {

        for (int i = 0; i < analyser->error_idx; i++) {
            analyser_error_print(analyser->errors[i]);
        }

        fprintf(stderr, "\nToo many analysis errors.\n");
        exit(1);
    }
    va_list args;
    va_start(args, format);

    analyser_error_t *error = arena_alloc(&analyser->ctx.arena, sizeof(analyser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = arena_alloc(&analyser->ctx.arena, msg_len);

    // Format the message
    vsnprintf(error->message, msg_len, format, args);
    va_end(args);

    // error->token = token;
    analyser->errors[analyser->error_idx++] = error;

    printf("\n(!) ANALYSER ERROR: %s\n", error->message);
    return false;
}
bool expect_type(analyser_t *analyser, ptype_t t, ptype_t expected, char* msg) {
    if (t != expected) {
        analyser_error_create(analyser, "%s. Expected type '%s' but got type %s", msg, primitive_type_to_str(expected), primitive_type_to_str(t));
        return false;
    }
    return true;
}

ptype_t annotate_statement(analyser_t *an, statement_t *s, scope_t *scope);
ptype_t annotate_block(analyser_t *an, block_expression_t *block, scope_t *scope) {
    ptype_t current_type = PTYPE_VOID;
    for (int i = 0; i < block->statement_count; i++) {
        ptype_t type = annotate_statement(an, &block->statements[i], scope);

        if (block->statements[i].type == EXPRESSION_TYPE_RETURN) {
            if (current_type == PTYPE_VOID) {
                current_type = type;
            } else {
                analyser_assert(current_type == type, an, "There are paths in this block evaluating to different types...");
            }
        }
    }

    return current_type;
}

ptype_t annotate_expression(analyser_t *an, expression_t *expression, scope_t *scope) {
    // if (expression->resultType != PTYPE_UNKNOWN) {
    //     return expression->resultType;
    // }

    switch (expression->type) {
        case EXPRESSION_TYPE_PREFIX: {
            ptype_t ltype = annotate_expression(an, expression->data.prefix.right, scope);
            expression->resultType = ltype;
            return ltype;
        }
        case EXPRESSION_TYPE_INFIX: {
            debug_expression(expression, 2);
            ptype_t ltype = annotate_expression(an, expression->data.infix.left, scope);
            printf("\nLtype is %s\n", primitive_type_to_str(ltype));
            ptype_t rtype = annotate_expression(an, expression->data.infix.right, scope);
            printf("\nRype is %s\n", primitive_type_to_str(rtype));
            expect_type(an, ltype, rtype, "Infix operations must be inbetween same types");
            expression->resultType = ltype;
            return ltype;
        }
        case EXPRESSION_TYPE_LITERAL: {
            switch (expression->data.literal.type) {
            case LITERAL_TYPE_I64: return PTYPE_I64;
            case LITERAL_TYPE_F64: return PTYPE_F64;
            case LITERAL_TYPE_CHARACTER: return PTYPE_CHAR;
            case LITERAL_TYPE_STRING: {
                    return PTYPE_ANY;
                };
            }
        }
        case EXPRESSION_TYPE_IDENTIFIER: {
            return expression->data.identifier.type;
        }
        case EXPRESSION_TYPE_FUNC_DECL: {
            expression_t* entry = scope_get_entry(scope, expression->data.func_decl.name);
            analyser_assert(entry == NULL, an, "Function '%s' has already been declared\n", expression->data.func_decl.name);

            scope_define_entry(scope, expression->data.func_decl.name, expression);

            if (expression->data.func_decl.value == NULL) {
                return expression->data.func_decl.type;
            }

            ptype_t type_hint = expression->data.func_decl.type;
            ptype_t inferred_type = annotate_expression(an, expression->data.func_decl.value, scope);

            if (type_hint == PTYPE_UNKNOWN) {
                type_hint = inferred_type;
                expression->data.func_decl.type = type_hint;
            }

            analyser_assert(!(inferred_type == PTYPE_UNKNOWN && type_hint == PTYPE_UNKNOWN), an, "Cannot infer the return type of function. Please add a type hint.");
            analyser_assert(inferred_type == type_hint, an, "Function does not return expected type in all paths");
            return inferred_type;
        }
        case EXPRESSION_TYPE_BLOCK: {
            return annotate_block(an, &expression->data.block, scope);
        }
        case EXPRESSION_TYPE_RETURN:
            return annotate_expression(an, expression->data.return_exp.expression, scope);
        case EXPRESSION_TYPE_CONDITIONAL: {
            ptype_t predicate_type = annotate_expression(an, expression->data.conditional.predicate, scope);
            expect_type(an, predicate_type, PTYPE_BOOL, "Conditional predicate should be of the bool type");

            ptype_t sbranch_type = annotate_expression(an, expression->data.conditional.success_branch, scope);
            if (expression->data.conditional.fail_branch != NULL) {
                ptype_t ebranch_type = annotate_expression(an, expression->data.conditional.fail_branch, scope);
                expect_type(an, sbranch_type, ebranch_type, "Conditional branches should return the same type");
            }
            return sbranch_type;
        }
        case EXPRESSION_TYPE_FOR:
            return PTYPE_UNKNOWN;
        case EXPRESSION_TYPE_STATIC_CALL: {
            ptype_t type = get_ptype_from_static_call(scope, expression);
            expression->resultType = type;
            return type;
        }
        case EXPRESSION_TYPE_CALL: {
            expression_t* entry = scope_get_entry(scope, expression->data.call.identifier_name);
            bool is_defined = analyser_assert(entry != NULL, an, "Function '%s' is not defined\n", expression->data.func_decl.name);
            if (!is_defined) return PTYPE_UNKNOWN;

            for (int i = 0; i < entry->data.func_decl.params.count; i++) {
                identifier_expression_t *identifier = vector_get(&entry->data.func_decl.params, i);
                expression_t *exp = vector_get(&expression->data.call.arguments, i);

                if (!analyser_assert(exp != NULL, an, "Too few arguments")) {
                    continue;
                }
                ptype_t exp_type = annotate_expression(an, exp, scope);

                analyser_assert(identifier->type == exp_type || identifier->type==PTYPE_ANY, an, "Argument does not match type");
            }

            // analyser_assert(entry == NULL, an, "Function '%s' is not defined\n", expression->data.func_decl.name);

            return PTYPE_UNKNOWN;
            // TODO use vtable
            // return annotate_block(an, expression->data.call.);
        }
        break;
    }

    return PTYPE_UNKNOWN;
};

ptype_t annotate_statement(analyser_t *an, statement_t *s, scope_t *scope) {
    switch (s->type) {
    case STATEMENT_TYPE_LET: {
        ptype_t type_hint = s->data.let.identifier.type;
        ptype_t inferred_type = annotate_expression(an, s->data.let.identifier.value, scope);

        if (type_hint == PTYPE_UNKNOWN) {
            s->data.let.identifier.type = inferred_type;
            return inferred_type;
        } else {
            expect_type(an, type_hint, inferred_type, "Type mismatch");
        }
        return PTYPE_UNKNOWN;
    }
    case STATEMENT_TYPE_DEFER:
        analyser_error_create(an, "Don't know how to analyse statement");
        break;
    case STATEMENT_TYPE_EXPRESSION:
        return annotate_expression(an, &s->data.expression, scope);
    }
    return PTYPE_UNKNOWN;
}

analyser_t analyser_new() {
    return (analyser_t) {
        .index = 0,
        .error_idx = 0,
        .ctx = {
            .arena = arena_new(1024*1024)
        }
    };
}

void analyse_program(program_t *program) {
    analyser_t a = analyser_new();
    printf("\nAnalyzing..\n");

    scope_t global_scope = (scope_t) {
        .table = hashmap_create(NULL),
        .upper = NULL
    };

    for (int i = 0; i < program->statements.count; i++) {
        statement_t *s = vector_get_ptr(&program->statements, i);
        s->data.expression.resultType = annotate_statement(&a, s, &global_scope);
        s->data.expression.resultType = PTYPE_CHAR;
    }


    if (a.error_idx > 0) {
        printf("\nAnalysis errors:\n");
        for (int i = 0; i < a.error_idx; i++) {
            analyser_error_print(a.errors[i]);
        }
    }
    printf("\nComplete anlaysis\n");
}
