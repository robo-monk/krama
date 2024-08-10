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

void scope_define_entry(scope_t *scope, char* name, expression_t* exp) {
    hashmap_insert(scope->table, name, exp);
}

scope_t scope_create_sub(Arena *arena, scope_t *upper) {
    return (scope_t) {
        .upper = upper,
        .table = hashmap_create(NULL),
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

bool type_generic_eq(type_t* a, type_t* b) {
    assert(a->kind != TYPE_KIND_UNKNOWN);
    assert(b->kind != TYPE_KIND_UNKNOWN);

    if (a->kind == TYPE_KIND_POINTER && b->kind == TYPE_KIND_POINTER) {
        return type_generic_eq(a->info.pointer, b->info.pointer);
    }

    if (a->kind == TYPE_KIND_GENERIC && b->kind == TYPE_KIND_PRIMITIVE) {
        return true;
    }

    if (b->kind == TYPE_KIND_GENERIC && a->kind == TYPE_KIND_PRIMITIVE) {
        return true;
    }


    if (a->kind == TYPE_KIND_PRIMITIVE && b->kind == TYPE_KIND_PRIMITIVE) {
        return strcmp(a->info.primitive, a->info.primitive) == 0;
    }

    return false;
}

bool type_eq(type_t* a, type_t* b) {
    assert(a->kind != TYPE_KIND_UNKNOWN);
    assert(b->kind != TYPE_KIND_UNKNOWN);

    if (a->kind == TYPE_KIND_POINTER && b->kind == TYPE_KIND_POINTER) {
        return type_eq(a->info.pointer, b->info.pointer);
    }

    if (a->kind == TYPE_KIND_PRIMITIVE && b->kind == TYPE_KIND_PRIMITIVE) {
        return strcmp(a->info.primitive, b->info.primitive) == 0;
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

        if (block->statements[i].type == STATEMENT_TYPE_EXPRESSION &&
            block->statements[i].data.expression.type == EXPRESSION_TYPE_RETURN
        ) {

            if (current_type.kind != TYPE_KIND_UNKNOWN) {
                analyser_assert(type_eq(&current_type, &type), an, "There are paths in this block evaluating to different types...");
            }
            current_type = type;
        }
    }

    return (current_type.kind == TYPE_KIND_UNKNOWN) ? (*(type_t*) (hashmap_get(an->ctx->types, "void"))) : current_type;
}

type_t get_ctype(analyser_t *an, char* typeid, bool is_ref) {
    assert(typeid != NULL);
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

// type can be either a literal or an "alias" aka identifier pointing to another type
type_t unwrap_expression_to_type(analyser_t *an, expression_t *typed, scope_t *scope) {
    if (typed->type == EXPRESSION_TYPE_LITERAL) {
        bool valid = analyser_assert(typed->data.literal.kind == LITERAL_KIND_TYPE, an, "type is a literal that does not contain valid type information");
        if (valid) {
            return typed->data.literal.data.type;
        }
    } else if (typed->type == EXPRESSION_TYPE_IDENTIFIER) {
        // type_t *type = scope_get_entry(an, scope, typed->data.identifier.name);
        printf("\nGET deftype '%s' \n", typed->data.identifier.name);
        expression_t *type_exp = scope_get_entry(scope, typed->data.identifier.name);
        printf("\n=> type is (%d)\n", type_exp->type);
        debug_expression(type_exp, 2);
        printf("---");
        bool valid = analyser_assert(type_exp != NULL, an, "aliased type is NULL")
                    && analyser_assert(type_exp->type == EXPRESSION_TYPE_LITERAL, an, "aliased type is not a type literal");

        if (valid) {
            return unwrap_expression_to_type(an, type_exp, scope);
        }
    }

    return (type_t) {
        .kind = TYPE_KIND_UNKNOWN
    };
}

type_t get_infix_ptype_result(analyser_t *an, infix_expression_t *infix, scope_t *scope) {
    switch (infix->operand.type) {
    case TOKEN_AS: {
        assert(infix->right != NULL);
        type_t ctype = unwrap_expression_to_type(an, infix->right, scope);
        analyser_assert(ctype.kind != TYPE_KIND_UNKNOWN, an, "Type is unknown");
        // assert(ctype.kind != TYPE_KIND_UNKNOWN, an, "Type is unknown");

        type_t ltype = annotate_expression(an, infix->left, scope);
        printf("[Analyser] CAST { ");
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


typedef bool (*type_eq_fn_t)(type_t* a, type_t* b);
bool compare_args_and_param_types(vector_t *args, vector_t *params, type_eq_fn_t type_eq_cb) {
    if (params->count != args->count) return false;
    for (int i = 0; i < args->count; i++) {
        expression_t *arg = vector_get_ptr(args, i);
        identifier_expression_t *param = vector_get_ptr(params, i);
        if (!type_eq_cb(&arg->resultType, &param->type)) {
            return false;
        }
    }
    return true;
}

const expression_t* get_func_variation(analyser_t *an, scope_t *scope, call_expression_t* call, type_eq_fn_t type_eq_fn) {
    while (scope != NULL && scope->table != NULL) {
        vector_t *entry = hashmap_get(scope->table, call->identifier_name);
        if (entry != NULL) {
            for (int i = 0; i < entry->count; i++) {
                expression_t *fn_decl = vector_get_ptr(entry, i);
                assert(fn_decl != NULL);
                assert(fn_decl->type == EXPRESSION_TYPE_FUNC_DECL);
                assert(strcmp(fn_decl->data.func_decl.name, call->identifier_name) == 0);
                bool match = compare_args_and_param_types(&call->arguments, &fn_decl->data.func_decl.params, type_eq_fn);
                if (match) {
                    return fn_decl;
                }
            }
        };
        scope = scope->upper;
    };
    return NULL;
}

vector_t* get_func_variations(scope_t *scope, char* identifier) {
    while (scope != NULL && scope->table != NULL) {
        vector_t *variations = hashmap_get(scope->table, identifier);
        if (variations != NULL) {
            return variations;
        };
        scope = scope->upper;
    };
    return NULL;
}

scope_t *get_global_scope(scope_t *scope) {
    while (scope->upper != NULL) {
        scope = scope->upper;
    };
    return scope;
}

vector_t *add_func_variation(analyser_t *an, expression_t *expression, scope_t *scope) {
    scope = get_global_scope(scope);
    assert(expression->type == EXPRESSION_TYPE_FUNC_DECL);
    vector_t *variations = get_func_variations(scope, expression->data.func_decl.name);

    if (variations == NULL) {
        printf("\n:: Initialise func declerations for '%s'\n", expression->data.func_decl.name);
        variations = arena_alloc(an->ctx->arena, sizeof(vector_t));
        vector_t variations_stack = vector_new(4, sizeof(expression_t*));
        memmove(variations, &variations_stack, sizeof(vector_t));
        hashmap_insert(scope->table, expression->data.func_decl.name, (vector_t*) variations);
    }
    printf("\nRegistering varation for '%s'\n", expression->data.func_decl.name);
    vector_push_ptr(variations, expression);
    assert(expression->data.func_decl.value != NULL);
    return variations;
}


expression_t* analyse_func_decl(analyser_t *an, expression_t *expression, scope_t *scope) {
    type_t type_hint = expression->data.func_decl.type;
    scope_t sub_scope = scope_create_sub(an->ctx->arena, scope);

    for (int i = 0; i<expression->data.func_decl.params.count; i++) {
        identifier_expression_t *argument = (identifier_expression_t*) vector_get_ptr(&expression->data.func_decl.params, i);

        expression_t *idexp = arena_alloc(an->ctx->arena, sizeof(expression_t));
        idexp->type = EXPRESSION_TYPE_IDENTIFIER;
        idexp->data.identifier = *argument;

        // printf("type is:: \n");
        // debug_type(&argument->type);
        // printf("---\n");
        scope_define_entry(&sub_scope, argument->name, idexp);
    }

    type_t inferred_type = annotate_expression(an, expression->data.func_decl.value, &sub_scope);
    if (type_hint.kind == TYPE_KIND_UNKNOWN || type_hint.kind == TYPE_KIND_GENERIC) {
        type_hint = inferred_type;
    }

    expression->data.func_decl.type = type_hint;
    analyser_assert(!(inferred_type.kind == TYPE_KIND_UNKNOWN && type_hint.kind == TYPE_KIND_UNKNOWN), an, "Cannot infer the return type of function. Please add a type hint.");
    analyser_assert(!(inferred_type.kind == TYPE_KIND_GENERIC && type_hint.kind == TYPE_KIND_GENERIC), an, "Generics are not supported.");

    printf("\n inferred type is:: ");
    debug_type(&inferred_type);
    printf("\n type hint is:: ");
    debug_type(&type_hint);

    analyser_assert(type_eq(&inferred_type, &type_hint), an, "Function '%s' does not return expected type in all paths", expression->data.func_decl.name);
    return expression;
}

expression_t *fn_variation_exp_dup(Arena *arena, expression_t *exp) {
    expression_t *generated = arena_alloc(arena, sizeof(expression_t));
    assert(exp != NULL);
    assert(exp->type == EXPRESSION_TYPE_FUNC_DECL);
    memcpy(generated, exp, sizeof(expression_t));
    generated->data.func_decl.name = arena_strdup(arena, exp->data.func_decl.name);
    return generated;
}


expression_t *generate_fn_implementation_for_args(analyser_t *an, const expression_t* fn_variation, vector_t *arguments, scope_t* scope) {
    scope_t subscope = scope_create_sub(an->ctx->arena, scope);

    expression_t *generated = fn_variation_exp_dup(an->ctx->arena, fn_variation);
    vector_t new_parms = vector_new(8, sizeof(identifier_expression_t*));

    assert(fn_variation->data.func_decl.params.count == arguments->count);

    for (int i = 0; i < fn_variation->data.func_decl.params.count; i++) {
        identifier_expression_t *param = (identifier_expression_t*) vector_get_ptr((vector_t *) &fn_variation->data.func_decl.params, i);
        expression_t *exp = (expression_t*) vector_get_ptr(arguments, i);

        assert(param != NULL);
        assert(param->type.kind != TYPE_KIND_UNKNOWN);
        // printf("\nparam -> %s::(%d)\n", param->name, param->type.kind);
        assert(param->type.kind >= 0 && param->type.kind < 15);
        identifier_expression_t *generated_param = arena_alloc(an->ctx->arena, sizeof(identifier_expression_t));
        memcpy(generated_param, param, sizeof(identifier_expression_t));

        if (param->type.kind == TYPE_KIND_GENERIC) {
            // assert(0);
            generated_param->type = exp->resultType;
            // expression_t type_def = (expression_t) {
            //     .type = EXPRESSION_TYPE_LITERAL,
            //     .data.literal = {
            //         .kind = LITERAL_KIND_TYPE,
            //         .data.type = exp->resultType
            //     }
            // };

            expression_t *type_def = arena_alloc(an->ctx->arena, sizeof(expression_t));
            type_def->type = EXPRESSION_TYPE_LITERAL;
            type_def->data.literal.kind = LITERAL_KIND_TYPE;
            type_def->data.literal.data.type = exp->resultType;
            // memcpy(type_def_ref, &type_def, sizeof(expression_t));

            printf("\ndeftype '%s' as ...\n", param->type.info.generic);
            debug_type(&type_def->data.literal.data.type);
            printf("---\n");
            expression_t *existing = scope_get_entry(&subscope, param->type.info.generic);
            if (existing != NULL) {
                type_t type = unwrap_expression_to_type(an, existing, scope);
                printf("\n existing type is:: ");
                debug_type(&type);
                printf("\n---");
                printf("\n overwrite type is:: ");
                debug_type(&exp->resultType);

                printf("\n---");

                assert(type_eq(&type, &exp->resultType));
            }

            scope_define_entry(&subscope, param->type.info.generic, type_def);
            // type_t* type = arena_alloc(an->ctx->arena, sizeof(type_t));
            // type->kind = TYPE_KIND_PRIMITIVE;
            // type->size = 8;
            // type->info.primitive = arena_strdup(an->ctx->arena, "brilliant");
            // printf("\n:: => %s\n", param->type.info.generic);
        }

        assert(type_eq(&generated_param->type, &exp->resultType));

        // printf("\n :: (%d ) ::> ", i);
        // debug_type(&generated_param->type);
        // printf("\n ---- \n");

        // vector_set_ptr(&generated->data.func_decl.params, i, generated_param);
        vector_push_ptr(&new_parms, generated_param);
    }

    generated->data.func_decl.params = new_parms;
    printf("\nreannoting function block value\n");
    analyse_func_decl(an, generated, &subscope);
    printf("\n\nInferred return type for this is:::: \n");
    debug_type(&generated->data.func_decl.type);
    printf("\n\n\n");
    return generated;
}



type_t annotate_expression(analyser_t *an, expression_t *expression, scope_t *scope) {
    // if (expression->resultType != PTYPE_UNKNOWN) {
    //     return expression->resultType;
    // }
    assert(expression != NULL);
    // printf("\n---> annotating & analysering.. \n");
    // debug_expression(expression, 1);
    // printf("\n");

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
            bool is_defined = analyser_assert(entry != NULL, an, "Identifier '%s' has not been declared\n", expression->data.identifier.name);
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
            // type_t t = annotate_func_decl(an, expression, scope);
            // type_t t = analyse_func_decl(an, expression, scope);
            add_func_variation(an, expression, scope);
            if (strcmp(expression->data.func_decl.name, "main") == 0) {
                analyse_func_decl(an, expression, scope);
                hashmap_insert(an->ctx->fn_declerations, expression->data.func_decl.name, expression);
            }
            return (type_t) { .kind = TYPE_KIND_UNKNOWN };
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
            printf("\nAnalysing CALL to '%s'\n", expression->data.call.identifier_name);
            bool invalid_args = false;
            // annotate arguments expressions
            for (int i = 0; i < expression->data.call.arguments.count; i++) {
                expression_t *arg = (expression_t*) vector_get_ptr(&expression->data.call.arguments, i);
                type_t exp_type = annotate_expression(an, arg, scope);
                arg->resultType = exp_type;
                invalid_args = !analyser_assert(exp_type.kind!=TYPE_KIND_UNKNOWN, an,
                    "Argument '%s' in call '%s' does not have a known type", arg->data.identifier.name, expression->data.call.identifier_name);
                if (invalid_args) break;
            }

            if (invalid_args) break;

            expression_t *decl = hashmap_get(an->ctx->fn_declerations, expression->data.call.identifier_name);
            if (decl != NULL) {
                assert(decl->type == EXPRESSION_TYPE_FUNC_DECL);
                return decl->data.func_decl.type;
            }

            const expression_t *fn_variation = get_func_variation(an, scope, &expression->data.call, type_eq);
            if (fn_variation == NULL) {
                printf("\nGetting generic varation for '%s'\n", expression->data.call.identifier_name);
                fn_variation = get_func_variation(an, scope, &expression->data.call, type_generic_eq);
            } else {
                printf("\nUsing explicit variation for '%s'\n", expression->data.call.identifier_name);
            }

            bool exists = analyser_assert(fn_variation != NULL, an,
                "There's no matching signature for call `%s`",
                expression->data.call.identifier_name);

            if (!exists) {
                printf("Available binding are..\n");
                scope = get_global_scope(scope);
                for (int i = 0; i < scope->table->keys->count; i++) {
                    // vector_t *variations =
                    char* key = vector_get_ptr(scope->table->keys, i);
                    vector_t* variations = hashmap_get(scope->table, key);
                    printf("(%d) %s with %d variatiosn\n", i, key, variations->count);
                    for (int o = 0; o < variations->count; o++) {
                        expression_t* exp = vector_get_ptr(variations, o);
                        assert(exp->type == EXPRESSION_TYPE_FUNC_DECL);
                        printf("    * %s\n", exp->data.func_decl.name);
                    }
                }
                assert(0);
                return (type_t) {
                    .kind = TYPE_KIND_UNKNOWN
                };
            }

            assert(fn_variation->type == EXPRESSION_TYPE_FUNC_DECL);

            expression_t *generated = generate_fn_implementation_for_args(an, fn_variation, &expression->data.call.arguments, scope);

            // char* mangled_name = an->ctx->fn_mangle(an->ctx, generated);
            char* mangled_name = fn_expr_name_mangle(an->ctx, generated);
            // char* mangled_name = generated->data.func_decl.name;
            printf("\n::Registering as %s\n", mangled_name);
            generated->data.func_decl.name = mangled_name;
            hashmap_insert(an->ctx->fn_declerations, mangled_name, generated);
            // hashmap_insert(an->ctx->fn_declerations, generated->data.func_decl.name, generated);
            expression->data.call.identifier_name = mangled_name;
            return generated->data.func_decl.type;
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
        printf("\n[Analyser]: Inferring '%s' Let decl : ", s->data.let.identifier.name);
        debug_type(&inferred_type);
        printf("\n");

        if (type_hint.kind == TYPE_KIND_UNKNOWN || type_hint.kind == TYPE_KIND_GENERIC) {
            s->data.let.identifier.type = inferred_type;
        } else {
            analyser_assert(type_eq(&type_hint, &inferred_type), an, "Type mismatch");
        }

        idexp->type = EXPRESSION_TYPE_IDENTIFIER;
        idexp->data.identifier = s->data.let.identifier;
        scope_define_entry(scope, s->data.let.identifier.name, idexp);

        analyser_assert(inferred_type.kind != TYPE_KIND_UNKNOWN, an, "Let decleration does not have a known type");

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
        printf("\n(%d) Analysing => ", i);
        statement_t *s = vector_get_ptr(&parser->program.statements, i);
        s->data.expression.resultType = annotate_statement(&a, s, &global_scope);
        printf("\n(%d) Done => ", i);
    }

    printf("\nDone\n");


    if (a.error_idx > 0) {
        printf("\nAnalysis errors:\n");
        for (int i = 0; i < a.error_idx; i++) {
            analyser_error_print(a.errors[i]);
        }
    }

    return a;
}
