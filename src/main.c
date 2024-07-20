#include <_ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokeniser.h"
#include "ast.h"
#include "parser.h"

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

    if (argc != 3) {
        printf("Argument mismatch.\nUsage: krama <input-file> <output-file>\n");
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
    token_t* tokens = malloc(MAX_TOKENS * sizeof(token_t));
    int token_count = tokenise(result.buffer, result.length, tokens);
    for (int i = 0; i < token_count; i ++) {
        token_debug(tokens[i]);
        printf("\n");
    }

    parse(tokens);

    free(tokens);
    file_read_result_free(result);
    return 0;
}
