#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

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
    for (size_t i = 0; i < program->statement_count; i++) {
        if (program->statements[i].type == STATEMENT_TYPE_BLOCK) {
            program_free((program_t *)&program->statements[i].data.block);
        }
    }
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


block_statement_t block_statement_new(void) {
    return (block_statement_t) {
        .statement_capacity = INITIAL_BLOCK_CAPACITY,
        .statement_count = 0,
        .statements = malloc(sizeof(statement_t) * INITIAL_BLOCK_CAPACITY)
    };
}

int block_add_statement(block_statement_t *block, statement_t statement) {
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
