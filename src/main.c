#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "compiler/compiler.h"
#include "frontend/parser.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void compile_file(const string filename) {
  // Start timing
  clock_t start_time = clock();

  // ANSI escape codes for colors and styles
  const char *color_green = "\033[32m";
  const char *color_yellow = "\033[33m";
  const char *color_reset = "\033[0m";
  const char *style_bold = "\033[1m";

  printf("\n%s%s > krama compile %s%s\n", style_bold, color_green, filename,
         color_reset);

  BlockStatement *program = program_from_filename(filename);
  printf("\ncompiling...\n");
  // ReturnValue ret_value = exec_program(program);
  compile_program(program, "out.c");

  // End timing
  clock_t end_time = clock();
  double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC *
                      1000; // Convert to milliseconds

  // Display result and execution time
  printf("%s---%s\n\n", color_yellow, color_reset);
  printf("%s > Done in %.4fms%s\n", color_green, time_spent, color_reset);
}
// Assuming these functions are defined elsewhere in your program
void run_file(const string filename) {
  // Start timing
  clock_t start_time = clock();

  // ANSI escape codes for colors and styles
  const char *color_green = "\033[32m";
  const char *color_yellow = "\033[33m";
  const char *color_reset = "\033[0m";
  const char *style_bold = "\033[1m";

  printf("\n%s%s > krama run %s%s\n", style_bold, color_green, filename,
         color_reset);

  printf("");
  BlockStatement *program = program_from_filename(filename);
  ReturnValue ret_value = exec_program(program);

  // End timing
  clock_t end_time = clock();
  double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC *
                      1000; // Convert to milliseconds

  // Display result and execution time
  printf("%s---%s\n\n", color_yellow, color_reset);
  printf("%d\n", ret_value.i32_value);
  printf("%s > Done in %.4fms%s\n", color_green, time_spent, color_reset);
}

int main(int argc, char *argv[]) {
  // Check if the only argument is "--help" or no arguments are passed
  if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    printf("usage:\n"
           "    krama run ./x.kr                        --execute a file\n"
           "    krama compile ./x.kr                    --execute a file\n"
           "    krama --help                            --show this\n");
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
  } else if (argc == 3 && strcmp(argv[1], "compile") == 0) {
    compile_file(argv[2]);
  } else {
    printf("Invalid usage. Use 'krama --help' for more information.\n");
  }

  return 0;
}
