#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "frontend/Parser.h"
#include "frontend/Tokeniser.h"
#include <stdio.h>
#include <string.h>

// Assuming these functions are defined elsewhere in your program
void run_file(const char *filename) {
  Token *tokens = malloc(512 * sizeof(Token));
  tokenise_file(filename, tokens);
  BlockStatement *program = parse_program(tokens);
  ReturnValue ret_value = exec_program(program);
  printf("---\n\n");
  printf("%d", ret_value.i32_value);
}
// void run_inline_string(const char *str);

int main(int argc, char *argv[]) {
  // Check if the only argument is "--help" or no arguments are passed
  if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    printf("usage:\n"
           "    krama run ./x.kr                 --execute a file\n"
           "    krama run \"inline code\"        --execute an inline string\n"
           "    krama --help                     --show this\n");
    return 0;
  }

  // Check for proper usage of "krama run" with an argument
  if (argc == 3 && strcmp(argv[1], "run") == 0) {
    if (argv[2][0] == '\"' || argv[2][0] == '\'') {
      // Assuming the inline string is passed with quotes, remove them
      // (simplified handling)
      char *inlineStr = argv[2] + 1;
      inlineStr[strlen(inlineStr) - 1] = '\0';
      // run_inline_string(inlineStr);
    } else {
      run_file(argv[2]);
    }
  } else {
    printf("Invalid usage. Use 'krama --help' for more information.\n");
  }

  return 0;
}
