#include "arena.h"
#include "analyser.h"
#include "ast.h"
#include "compiler.h"
#include "hashmap.h"
#include "parser.h"
#include "tokeniser.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


void debug_type(type_t* t) {
    assert(t != NULL);
    switch (t->kind) {
        case TYPE_KIND_PRIMITIVE:
            printf("[Type] PRIMITIVE `%s` with size %zu", t->info.primitive, t->size);
        break;
        case TYPE_KIND_POINTER: {
            printf("[Type] POINTER to ");
            debug_type(t->info.pointer);
        }
        break;
        case TYPE_KIND_UNKNOWN:
        printf("[Type] UNKNOWN");
        break;
    }
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

    analyser_error_t *error = arena_alloc(analyser->ctx->arena, sizeof(analyser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = arena_alloc(analyser->ctx->arena, msg_len);

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

    analyser_error_t *error = arena_alloc(analyser->ctx->arena, sizeof(analyser_error_t));

    // Allocate memory for the message
    int msg_len = vsnprintf(NULL, 0, format, args) + 1;
    error->message = arena_alloc(analyser->ctx->arena, msg_len);

    // Format the message
    vsnprintf(error->message, msg_len, format, args);
    va_end(args);

    // error->token = token;
    analyser->errors[analyser->error_idx++] = error;

    printf("\n(!) ANALYSER ERROR: %s\n", error->message);
    return false;
}

char* type_get_primitive(type_t *t) {
    if (t->kind == TYPE_KIND_PRIMITIVE) {
        return t->info.primitive;
    } else if (t->kind == TYPE_KIND_POINTER) {
        return type_get_primitive(t->info.pointer);
    } else {
        assert(0);
        return "(unknown)";
    }
}

bool type_eq(type_t* a, type_t* b) {
    assert(a->kind != TYPE_KIND_UNKNOWN);
    assert(b->kind != TYPE_KIND_UNKNOWN);

    if (a->kind == TYPE_KIND_POINTER && b->kind == TYPE_KIND_POINTER) {
        return type_eq(a->info.pointer, b->info.pointer);
    }

    if (a->kind == TYPE_KIND_PRIMITIVE && b->kind == TYPE_KIND_PRIMITIVE) {
        return strcmp(a->info.primitive, a->info.primitive) == 0;
    }

    return false;
}

type_t annotate_statement(analyser_t *an, statement_t *s, scope_t *scope);
type_t annotate_expression(analyser_t *an, expression_t *expression, scope_t *scope);
type_t annotate_block(analyser_t *an, block_expression_t *block, scope_t *scope) {
    type_t current_type = (type_t) { .kind = TYPE_KIND_UNKNOWN };
    scope_t sub_scope = scope_create_sub(an->ctx->arena, scope);

    for (int i = 0; i < block->statement_count; i++) {
        type_t type = annotate_statement(an, &block->statements[i], &sub_scope);
        // printf("%d ==> \n", i);
        // debug_type(&type);

        if (block->statements[i].type == STATEMENT_TYPE_EXPRESSION &&
            block->statements[i].data.expression.type == EXPRESSION_TYPE_RETURN
        ) {
            // if (!type_is(&type, "void")) {
            if (current_type.kind != TYPE_KIND_UNKNOWN) {
                analyser_assert(type_eq(&current_type, &type), an, "There are paths in this block evaluating to different types...");
            }
            current_type = type;
        }
    }

    return (current_type.kind == TYPE_KIND_UNKNOWN) ? (*(type_t*) (hashmap_get(an->ctx->types, "void"))) : current_type;
}

type_t get_ctype(analyser_t *an, char* typeid, bool is_ref) {
    type_t *type_info = hashmap_get(an->ctx->types, typeid);
    assert(type_info != NULL);
    if (is_ref) {
        return (type_t) {
            .kind = TYPE_KIND_POINTER,
            .info.pointer = type_info
        };
    } else {
        return *type_info;
    }
}

type_t get_prefix_ptype_result(analyser_t *an, prefix_expression_t *prefix, scope_t *scope) {
    switch (prefix->operand.type) {
    case TOKEN_ASTERISK:{
        type_t rtype = annotate_expression(an, prefix->right, scope);
        printf("\n\n\n");
        debug_type(&rtype);
        printf("...");
        analyser_assert(rtype.kind == TYPE_KIND_POINTER, an, "Cannnot dereference a non pointer");
        analyser_assert(rtype.info.pointer != NULL, an, "Pointer to invalid type");
        prefix->right->resultType = *rtype.info.pointer;
        return prefix->right->resultType;
    }
    case TOKEN_BANG:{
        return get_ctype(an, "bool", false);
    }
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    {
        type_t ltype = annotate_expression(an, prefix->right, scope);
        prefix->right->resultType = ltype;
        return ltype;
    }
    default:
        break;
    }
    analyser_assert(0, an, "Invalid prefix operation");
    return (type_t) { .kind = TYPE_KIND_UNKNOWN };
}
type_t get_infix_ptype_result(analyser_t *an, infix_expression_t *infix, scope_t *scope) {
    switch (infix->operand.type) {
    case TOKEN_AS: {
        assert(infix->right != NULL);
        analyser_assert(infix->right->type == EXPRESSION_TYPE_LITERAL, an, "RHS of a cast should be a literal type");
        analyser_assert(infix->right->data.literal.kind == LITERAL_KIND_TYPE, an, "RHS of a cast should be a type. Literal is not type");
        type_t ctype = infix->right->data.literal.data.type;
        analyser_assert(ctype.kind != TYPE_KIND_UNKNOWN, an, "Type is unknown");

        type_t ltype = annotate_expression(an, infix->left, scope);
        printf("\nCAST { ");
        debug_type(&ltype);
        printf(" } to { ");
        debug_type(&ctype);
        printf("}\n");
        infix->left->resultType = ctype;
        return ctype;
    }
    case TOKEN_EQ:
    case TOKEN_LT:
    case TOKEN_GT:
    case TOKEN_NEQ:
    case TOKEN_EQEQ:
    case TOKEN_LTE:
    case TOKEN_GTE: {
        // TODO: is_op_defined_for_args(...)
        type_t ltype = annotate_expression(an, infix->left, scope);
        type_t rtype = annotate_expression(an, infix->right, scope);

        analyser_assert(type_eq(&ltype, &rtype), an, "Infix operations must be inbetween same types");
        return get_ctype(an, "bool", false);
    }
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_SLASH:
    case TOKEN_ASTERISK: {
        // TODO: is_op_defined_for_args(...)
        type_t ltype = annotate_expression(an, infix->left, scope);
        type_t rtype = annotate_expression(an, infix->right, scope);
        analyser_assert(type_eq(&ltype, &rtype), an, "Infix operations must be inbetween same types");
        return ltype;
    }
    default:
        break;
    }
    analyser_assert(0, an, "Invalid infix operation");
    return (type_t) { .kind = TYPE_KIND_UNKNOWN };
}

type_t annotate_expression(analyser_t *an, expression_t *expression, scope_t *scope) {
    // if (expression->resultType != PTYPE_UNKNOWN) {
    //     return expression->resultType;
    // }
    assert(expression != NULL);
    switch (expression->type) {
        case EXPRESSION_TYPE_PREFIX: {
            // type_t ltype = annotate_expression(an, expression->data.prefix.right, scope);
            type_t ltype = get_prefix_ptype_result(an, &expression->data.prefix, scope);
            expression->resultType = ltype;
            return ltype;
        }
        case EXPRESSION_TYPE_INFIX: {
            type_t t = get_infix_ptype_result(an, &expression->data.infix, scope);

            expression->resultType = t;
            return t;
        }
        case EXPRESSION_TYPE_LITERAL: {
            switch (expression->data.literal.kind) {
            case LITERAL_KIND_I64: return get_ctype(an, "i64", false);
            case LITERAL_KIND_F64: return get_ctype(an, "f64", false);
            case LITERAL_KIND_CHARACTER: return get_ctype(an, "char", false);
            case LITERAL_KIND_TYPE: return expression->data.literal.data.type;
            case LITERAL_KIND_STRING: {
                    return get_ctype(an, "char", true);
                };
            }
            printf("\n unsupported literal?\n");
            assert(0);
        }
        case EXPRESSION_TYPE_IDENTIFIER_ASSIGNMENT: {
            expression_t* entry = scope_get_entry(scope, expression->data.identifier.name);
            bool is_defined = analyser_assert(entry != NULL, an, "Identifier '%s' is not declared\n", expression->data.identifier.name);
            if (!is_defined) return (type_t) { .kind = TYPE_KIND_UNKNOWN };
            bool is_identifier = analyser_assert(entry->type == EXPRESSION_TYPE_IDENTIFIER, an, "'%s' is not an identifier", expression->data.identifier.name);
            if (!is_identifier) {
                printf("\n it is... %d not .. %d\n",entry->type, EXPRESSION_TYPE_IDENTIFIER);
            }
            type_t ass_type = annotate_expression(an, expression->data.identifier.value, scope);
            analyser_assert(type_eq(&entry->data.identifier.type, &ass_type), an, "Assigment expression does not match identifier type!");
            return ass_type;
        }
        case EXPRESSION_TYPE_IDENTIFIER: {
            expression_t* entry = scope_get_entry(scope, expression->data.identifier.name);
            bool is_defined = analyser_assert(entry != NULL, an, "Identifier '%s' is not declared\n", expression->data.identifier.name);
            if (!is_defined) return (type_t) { .kind = TYPE_KIND_UNKNOWN };
            bool is_identifier = analyser_assert(entry->type == EXPRESSION_TYPE_IDENTIFIER, an, "'%s' is not an identifier", expression->data.identifier.name);
            if (!is_identifier) {
                printf("\n it is... %d not .. %d\n",entry->type, EXPRESSION_TYPE_IDENTIFIER);
            }
            return entry->data.identifier.type;
        }
        case EXPRESSION_TYPE_EXTERN_FUNC_DECL: {
            scope_define_entry(scope, expression->data.func_decl.name, expression);
            return expression->data.func_decl.type;
        }
        case EXPRESSION_TYPE_FUNC_DECL: {
            expression_t* entry = scope_get_entry(scope, expression->data.func_decl.name);
            analyser_assert(entry == NULL, an, "Function '%s' has already been declared\n", expression->data.func_decl.name);

            assert(expression->data.func_decl.value != NULL);

            type_t type_hint = expression->data.func_decl.type;
            scope_t sub_scope = scope_create_sub(an->ctx->arena, scope);

            for (int i = 0; i<expression->data.func_decl.params.count; i++) {
                identifier_expression_t *id = (identifier_expression_t*) vector_get_ptr(&expression->data.func_decl.params, i);
                expression_t *idexp = arena_alloc(an->ctx->arena, sizeof(expression_t));
                idexp->type = EXPRESSION_TYPE_IDENTIFIER;
                idexp->data.identifier = *id;
                scope_define_entry(&sub_scope, id->name, idexp);
            }


            type_t inferred_type = annotate_expression(an, expression->data.func_decl.value, &sub_scope);
            if (type_hint.kind == TYPE_KIND_UNKNOWN) {
                type_hint = inferred_type;
            }

            expression->data.func_decl.type = type_hint;
            expression->data.func_decl.name = an->ctx->fn_mangle(an->ctx, expression);

            scope_define_entry(scope, expression->data.func_decl.name, expression);
            analyser_assert(!(inferred_type.kind == TYPE_KIND_UNKNOWN && type_hint.kind == TYPE_KIND_UNKNOWN), an, "Cannot infer the return type of function. Please add a type hint.");
            analyser_assert(type_eq(&inferred_type, &type_hint), an, "Function '%s' does not return expected type in all paths", expression->data.func_decl.name);
            return type_hint;
        }
        case EXPRESSION_TYPE_BLOCK: {
            type_t type = annotate_block(an, &expression->data.block, scope);
            return type;
        }
        case EXPRESSION_TYPE_RETURN: {
            return annotate_expression(an, expression->data.return_exp.expression, scope);
        }
        case EXPRESSION_TYPE_CONDITIONAL: {
            type_t predicate_type = annotate_expression(an, expression->data.conditional.predicate, scope);
            type_t bool_type = get_ctype(an, "bool", false);
            analyser_assert(type_eq(&predicate_type, &bool_type), an, "Conditional predicate should be of the bool type");

            type_t sbranch_type = annotate_expression(an, expression->data.conditional.success_branch, scope);
            if (expression->data.conditional.fail_branch != NULL) {
                type_t ebranch_type = annotate_expression(an, expression->data.conditional.fail_branch, scope);
                analyser_assert(type_eq(&sbranch_type, &ebranch_type), an, "Conditional branches should return the same type");
            }
            return sbranch_type;
        }
        case EXPRESSION_TYPE_FOR:
            assert(0);
            return (type_t) { .kind = TYPE_KIND_UNKNOWN };
        case EXPRESSION_TYPE_STATIC_CALL: {
            for (int i = 0; i < expression->data.call.arguments.count; i++) {
                expression_t *exp = (expression_t*) vector_get_ptr(&expression->data.call.arguments, i);
                type_t exp_type = annotate_expression(an, exp, scope);
                exp->resultType = exp_type;
                analyser_assert(exp_type.kind!=TYPE_KIND_UNKNOWN, an, "Parameters should have well defined types");
            }
            return get_ctype(an, "any", false);
        }
        case EXPRESSION_TYPE_CALL: {
             // annotate arguments expressions
            for (int i = 0; i < expression->data.call.arguments.count; i++) {
                expression_t *exp = (expression_t*) vector_get_ptr(&expression->data.call.arguments, i);
                type_t exp_type = annotate_expression(an, exp, scope);
                exp->resultType = exp_type;
                analyser_assert(exp_type.kind!=TYPE_KIND_UNKNOWN, an, "Parameters should have well defined types");
            }

            // check for extern functions (skip mangle)
            expression_t* unmangled_entry = scope_get_entry(scope, expression->data.call.identifier_name);
            if (unmangled_entry != NULL && unmangled_entry->type == EXPRESSION_TYPE_EXTERN_FUNC_DECL) {
                printf("\n skipping mangle for... %s\n", expression->data.call.identifier_name);
                return unmangled_entry->data.func_decl.type;
            }


            expression->data.call.identifier_name = an->ctx->fn_mangle(an->ctx, expression);
            expression_t* entry = scope_get_entry(scope, expression->data.call.identifier_name);

            bool is_defined = analyser_assert(entry != NULL, an, "Function signature '%s' is not defined\n", expression->data.func_decl.name);

            if (!is_defined) {
                return (type_t) { .kind = TYPE_KIND_UNKNOWN };
            }

            return entry->data.identifier.type;
        }
        break;
    }

    return (type_t) { .kind = TYPE_KIND_UNKNOWN };
};

type_t annotate_statement(analyser_t *an, statement_t *s, scope_t *scope) {
    switch (s->type) {
    case STATEMENT_TYPE_LET: {

        expression_t* entry = scope_get_entry(scope, s->data.let.identifier.name);
        analyser_assert(entry == NULL, an, "Identifier '%s' has already been declared\n", s->data.let.identifier.name);
        analyser_assert(s->data.let.identifier.value != NULL, an, "Unitialised identifier '%s' is not allowed", s->data.let.identifier.name);

        expression_t *idexp = arena_alloc(an->ctx->arena, sizeof(expression_t));

        type_t type_hint = s->data.let.identifier.type;
        type_t inferred_type = annotate_expression(an, s->data.let.identifier.value, scope);

        if (type_hint.kind == TYPE_KIND_UNKNOWN) {
            s->data.let.identifier.type = inferred_type;
        } else {
            analyser_assert(type_eq(&type_hint, &inferred_type), an, "Type mismatch");
        }

        idexp->type = EXPRESSION_TYPE_IDENTIFIER;
        idexp->data.identifier = s->data.let.identifier;
        scope_define_entry(scope, s->data.let.identifier.name, idexp);
        return inferred_type;
    }
    case STATEMENT_TYPE_DEFER:
        analyser_error_create(an, "Don't know how to analyse statement");
        break;
    case STATEMENT_TYPE_EXPRESSION:
        return annotate_expression(an, &s->data.expression, scope);
    }
    return (type_t) { .kind = TYPE_KIND_UNKNOWN };
}

analyser_t analyser_new(CompilerContext *ctx) {
    return (analyser_t) {
        .index = 0,
        .error_idx = 0,
        .ctx = ctx
    };
}

analyser_t analyse_program(parser_t *parser, CompilerContext *ctx) {
    analyser_t a = analyser_new(ctx);

    printf("\nAnalyzing..\n");
    scope_t global_scope = (scope_t) {
        .table = hashmap_create(NULL),
        .upper = NULL
    };

    for (int i = 0; i < parser->program.statements.count; i++) {
        statement_t *s = vector_get_ptr(&parser->program.statements, i);
        s->data.expression.resultType = annotate_statement(&a, s, &global_scope);
    }


    if (a.error_idx > 0) {
        printf("\nAnalysis errors:\n");
        for (int i = 0; i < a.error_idx; i++) {
            analyser_error_print(a.errors[i]);
        }
    }

    return a;
}
