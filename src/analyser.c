#include "arena.h"
#include "analyser.h"
#include "ast.h"
#include "hashmap.h"
#include "parser.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <stdbool.h>

// typedef struct scope_t {
//     hashmap_t *table;
//     struct scope_t *upper;
// } scope_t;

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
}

bool expect_type(analyser_t *analyser, ptype_t t, ptype_t expected, char* msg) {
    if (t != expected) {
        analyser_error_create(analyser, "%s. Expected type '%s' but got type %s", msg, primitive_type_to_str(expected), primitive_type_to_str(t));
        return false;
    }
    return true;
}

ptype_t annotate_statement(analyser_t *an, statement_t *s);

ptype_t annotate_expression(analyser_t *an, expression_t *expression) {
    // printf("\n---> hello? %d\n", expression->type);
    // debug_expression(expression, 5);
    // if (expression->resultType != PTYPE_UNKNOWN) {
    //     return expression->resultType;
    // }

    switch (expression->type) {
        case EXPRESSION_TYPE_PREFIX: {
            ptype_t ltype = annotate_expression(an, expression->data.infix.left);
            expression->resultType = ltype;
            return ltype;
        }
        case EXPRESSION_TYPE_INFIX: {
            printf("-> here we go\n");
            ptype_t ltype = annotate_expression(an, expression->data.infix.left);
            printf("\nLtype is %s\n", primitive_type_to_str(ltype));
            ptype_t rtype = annotate_expression(an, expression->data.infix.right);
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
                    analyser_error_create(an, "Strings are not supported\n");
                    return PTYPE_UNKNOWN;
                };
            }
        }
        case EXPRESSION_TYPE_IDENTIFIER: {
            return expression->data.identifier.type;
        }
        case EXPRESSION_TYPE_FUNC_DECL:
            break;
        case EXPRESSION_TYPE_BLOCK: {
            return annotate_statement(an, &expression->data.block.statements[
                expression->data.block.statement_count-1
            ]);
        }
        case EXPRESSION_TYPE_RETURN:
        case EXPRESSION_TYPE_CONDITIONAL: {
            ptype_t predicate_type = annotate_expression(an, expression->data.conditional.predicate);
            expect_type(an, predicate_type, PTYPE_BOOL, "Conditional predicate should be of the bool type");

            ptype_t sbranch_type = annotate_expression(an, expression->data.conditional.success_branch);
            if (expression->data.conditional.fail_branch != NULL) {
                ptype_t ebranch_type = annotate_expression(an, expression->data.conditional.fail_branch);
                expect_type(an, sbranch_type, ebranch_type, "Conditional branches should return the same type");
            }
            return sbranch_type;
        }
        case EXPRESSION_TYPE_FOR:
        case EXPRESSION_TYPE_CALL:
        break;
    }

    return PTYPE_UNKNOWN;
};

ptype_t annotate_statement(analyser_t *an, statement_t *s) {
    switch (s->type) {
    case STATEMENT_TYPE_LET: {
        ptype_t type_hint = s->data.let.identifier.type;
        ptype_t inferred_type = annotate_expression(an, s->data.let.identifier.value);

        // expect_type(an, type_hint, inferred_type, "expected types here behih");

        if (type_hint == PTYPE_UNKNOWN) {
            s->data.let.identifier.value->resultType = inferred_type;
        } else {
            expect_type(an, type_hint, inferred_type, "Type mismatch");
        }
        return PTYPE_UNKNOWN;
    }
    case STATEMENT_TYPE_DEFER:
        analyser_error_create(an, "Don't know how to analyse statement");
        break;
    case STATEMENT_TYPE_EXPRESSION:
        return annotate_expression(an, &s->data.expression);
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

    for (int i = 0; i < program->statement_count; i++) {
        statement_t s = program->statements[i];
        annotate_statement(&a, &s);
    }

    if (a.error_idx > 0) {
            printf("\nAnalysis errors:\n");
            for (int i = 0; i < a.error_idx; i++) {
                analyser_error_print(a.errors[i]);
            }
        }
}
