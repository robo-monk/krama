#include <stddef.h>
#define ARENA_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION

#include "analyser.h"
#include "arena.h"
#include "hashmap.h"
#include "compiler.h"
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokeniser.h"
#include "ast.h"
#include "parser.h"
#include "compiler.h"

typedef struct {
    int length;
    const char* buffer;
} file_read_result_t;

file_read_result_t read_file_to_str(const char* filename) {
    char *buffer = 0;
    int length = -1;
    FILE *f = fopen(filename, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    }

    return (file_read_result_t) {
        .length = length,
        .buffer = buffer
    };
}

void file_read_result_free(file_read_result_t res) {
    free((void*) res.buffer);
}

#define MAX_TOKENS 4096
int main(int argc, char *argv[]) {
    // hashmap_t hash = hashmap_create(sizeof(char*));

    // hashmap_insert(&hash, "set", "hello there bing bong");
    // hashmap_insert(&hash, "t", "yes ackualh");
    // printf("naah %s\n", (char*) hashmap_get(&hash, "t"));
    // printf("yes please %s\n", (char*) hashmap_get(&hash, "set"));

    // hashmap_free(&hash);
    // return 0;

    if (argc == 1) {
        char inp[1024];
        printf("Initialising REPL... Type `exit` to exit\n");
        parser_t parser = parser_new();
        while (true) {
            printf("> ");
            if (fgets(inp, sizeof(inp), stdin) == NULL) {
                break;  // Handle EOF
            }

            // inp[strcspn(inp, "\n")] = ';';

            if (strcasecmp(inp, "exit\n") == 0) {
                break;
            }

            int len = strlen(inp);

            token_t* tokens = malloc(MAX_TOKENS * sizeof(token_t));
            int token_count = tokenise(inp, len, tokens);

            // printf("inp is:: `%s`\n", inp);
            for (int i = 0; i < token_count; i ++) {
                printf("->");
                token_debug(tokens[i]);
                printf("\n");
            }
            parse(&parser, tokens);
            free(tokens);
        }

        parser_destroy(&parser);
        return 0;
    } else if (argc == 3) {
        printf("Compiling...\n");
    } else {
        printf("Argument mismatch.\nUsage: krama <input-file> <output-file> or krama to initialise the REPL\n");
        return 1;
    }

    char* input_file = argv[1];
    char* output_file = argv[2];


    file_read_result_t result = read_file_to_str(input_file);

    if (result.length == -1) {
        printf("could not read file\n");
        return 1;
    }

    // TODO use dynamic array here
    // token_t* tokens = malloc(MAX_TOKENS * sizeof(token_t));
    vector_t tokens = tokenise2(result.buffer, result.length);
    for (int i = 0; i < tokens.count; i ++) {
        token_t *t = vector_get(&tokens, i);
        token_debug(*t);
        printf("\n");
    }

    token_t* token_array = vector_to_array(&tokens);
    vector_free(&tokens);

    parser_t parser = parser_new();
    parse(&parser, token_array);

    if (parser.error_idx > 0) {
        exit(1);
    }

    analyse_program(&parser);
    compile(&parser, output_file);

    parser_destroy(&parser);
    free(token_array);
    file_read_result_free(result);

    return 0;
}
