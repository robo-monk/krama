#include "ast.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_PROGRAM_CAPACITY 64
#define INITIAL_BLOCK_CAPACITY 1


program_t program_create(void) {
    return (program_t) {
        .statement_capacity = INITIAL_PROGRAM_CAPACITY,
        .statement_count = 0,
        .statements = malloc(sizeof(statement_t) * INITIAL_PROGRAM_CAPACITY)
    };
}

void program_free(program_t *program) {
    if (program == NULL) {
        return;
    }
    // Free any dynamically allocated memory in statements if necessary
    // for (size_t i = 0; i < program->statement_count; i++) {
    //     if (program->statements[i].type == EXPRESSION_TYPE_BLOCK) {
    //         program_free((program_t *)&program->statements[i].data.expression.);
    //     }
    // }
    free(program->statements);
    free(program);
}

int program_add_statement(program_t *program, statement_t statement) {
    if (program->statement_count == program->statement_capacity) {
        program->statement_capacity *= 2;
        program->statements = realloc(program->statements, program->statement_capacity * sizeof(statement_t));
        if (program->statements == NULL) {
            printf("Allocation failed.");
            return -1;
        }
    }

    program->statements[program->statement_count++] = statement;
    return 0;
}


block_expression_t block_expression_new(void) {
    return (block_expression_t) {
        .statement_capacity = INITIAL_BLOCK_CAPACITY,
        .statement_count = 0,
        .statements = malloc(sizeof(statement_t) * INITIAL_BLOCK_CAPACITY)
    };
}

int block_add_statement(block_expression_t *block, statement_t statement) {
    if (block->statement_count == block->statement_capacity) {
        block->statement_capacity *= 2;
        block->statements = realloc(block->statements, block->statement_capacity * sizeof(statement_t));
        if (block->statements == NULL) {
            printf("Block Allocation failed.");
            return -1;
        }
    }

    block->statements[block->statement_count++] = statement;
    return 0;
}



void add_tabs(int count) {
    for (int i = 0; i < count; i++) {
        printf("⎹");
        printf("    ");
    }
}

void statement_debug(statement_t *s, int ident);

void debug_expression(expression_t *expression, int ident) {
    add_tabs(ident);
    if (expression == NULL) {
        printf("NULL");
        return;
    }
    // printf("%s", ident);
    switch (expression->type) {
    case EXPRESSION_TYPE_PREFIX:
        printf("expr PREFIX (");
        token_debug(expression->data.prefix.operand);
        printf(")\n");
        // add_tabs(ident+1);
        // printf("R: \n");
        debug_expression(expression->data.prefix.right, ident+1);
        break;
    case EXPRESSION_TYPE_INFIX:
        printf("expr INFIX (");
        token_debug(expression->data.infix.operand);
        printf(")\n");
        add_tabs(ident);
        printf("R:\n");
        debug_expression(expression->data.infix.left, ident+1);
        printf("\n");
        add_tabs(ident);
        printf("L:\n");
        debug_expression(expression->data.infix.right, ident+1);
        break;
    case EXPRESSION_TYPE_LITERAL:
        // add_tabs(ident);
        printf("expr LITERAL (%ld)", expression->data.literal.data.i64);
        break;
    case EXPRESSION_TYPE_IDENTIFIER:
        printf("expr IDENTIFIER (%s)", expression->data.identifier.name);;
        break;
    case EXPRESSION_TYPE_BLOCK: {
        // add_tabs(ident);
        printf("block {\n");
        for (int i = 0; i < expression->data.block.statement_count; i++) {
            add_tabs(ident+1);
            printf("#%d\n", i);
            statement_debug(&expression->data.block.statements[i], ident+1);
            printf("\n");
        }
        add_tabs(ident);
        printf("}");
        break;
    }
    case EXPRESSION_TYPE_FUNC_DECL: {
        printf("FUNCTION %s\n", expression->data.func_decl.name);
        debug_expression(expression->data.func_decl.value, ident+1);
        break;
    }
    case EXPRESSION_TYPE_CONDITIONAL: {
        printf("expr IF\n");

        add_tabs(ident);
        printf("predicate:\n");
        debug_expression(expression->data.conditional.predicate, ident+1);

        printf("\n");

        add_tabs(ident);
        printf("success:\n");
        debug_expression(expression->data.conditional.success_branch, ident+1);

        printf("\n");
        add_tabs(ident);
        printf("fail:\n");
        debug_expression(expression->data.conditional.fail_branch, ident+1);
        break;
    }
    case EXPRESSION_TYPE_RETURN: {
        printf("expr RETURN\n");
        debug_expression(expression->data.return_exp.expression, ident+1);
    }
    default:
        printf("\n not implemented ?? \n");
    }
}


void statement_debug(statement_t *s, int ident) {
    switch (s->type) {
    case STATEMENT_TYPE_LET: {
        add_tabs(ident);
        printf("LET `%s` = ", s->data.let.identifier.name);
        printf("\n");
        // add_tabs(ident);
        // printf("└─ ");
        return debug_expression(s->data.let.identifier.value, ident+1);
    }
    case STATEMENT_TYPE_EXPRESSION: {
        return debug_expression(&s->data.expression, ident);
    }
    default:
        printf("\nnot implemented\n");
        break;
    }

}
