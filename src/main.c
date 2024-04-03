#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "compiler/compiler.h"
#include "frontend/parser.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"
#define STYLE_BOLD "\033[1m"

// A function to print messages with color
void print_colored(const char *color, const char *message) {
  printf("%s%s%s\n", color, message, COLOR_RESET);
}

// Global variable for standard library path
char *global_std_path = "std.kr";

void compile_file_no_std(const string filename, const string out_filename) {
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
  compile_program(program, out_filename);

  // End timing
  clock_t end_time = clock();
  double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC *
                      1000; // Convert to milliseconds

  // Display result and execution time
  printf("%s---%s\n\n", color_yellow, color_reset);
  printf("%s > Done in %.4fms%s\n", color_green, time_spent, color_reset);
}
void compile_file(const string filename, const string std_filename,
                  const string out_filename) {
  // Start timing
  clock_t start_time = clock();

  // ANSI escape codes for colors and styles
  const char *color_green = "\033[32m";
  const char *color_yellow = "\033[33m";
  const char *color_reset = "\033[0m";
  const char *style_bold = "\033[1m";

  printf("\n%s%s > krama compile %s%s\n", style_bold, color_green, filename,
         color_reset);

  BlockStatement *std = program_from_filename(std_filename);
  BlockStatement *program = program_from_filename(filename);
  printf("\ncompiling...\n");
  // ReturnValue ret_value = exec_program(program);
  compile_program_with_std(program, std, out_filename);

  // End timing
  clock_t end_time = clock();
  double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC *
                      1000; // Convert to milliseconds

  // Display result and execution time
  printf("%s---%s\n\n", color_yellow, color_reset);
  printf("%s > Done in %.4fms%s\n", color_green, time_spent, color_reset);
}

typedef struct {
  string *input_files;
  string output_file;
  string std_path;
  bool no_std;
} Arguments;

typedef struct {
  bool exit;
  int skip_ahead;
} ArgumentParseResult;

typedef ArgumentParseResult (*ArgumentAction)(Arguments *, string *argv,
                                              int current_idx);

ArgumentParseResult show_help(Arguments *args, string *argv, int current_idx) {
  printf("\nusage:\n"
         "    krama run ./x.kr                        --execute a file\n"
         "    krama compile ./x.kr                    --execute a file\n"
         "    krama --help                            --show this\n");
  return (ArgumentParseResult){.exit = true, .skip_ahead = 0};
}

ArgumentParseResult no_std(Arguments *args, string *argv, int current_idx) {
  args->no_std = true;
  return (ArgumentParseResult){.exit = false, .skip_ahead = 0};
}

ArgumentParseResult def_std_path(Arguments *args, string *argv,
                                 int current_idx) {
  string filename = argv[current_idx];
  if (filename[0] == '-') {
    printf("\n expected filename");
    exit(0);
  }
  args->std_path = filename;
  return (ArgumentParseResult){.skip_ahead = 1, .exit = false};
}

ArgumentParseResult compile(Arguments *args, string *argv, int current_idx) {
  string filename = argv[current_idx];
  if (filename[0] == '-') {
    printf("\n expected input filename");
    exit(0);
  }
  args->input_files[0] = filename;

  string outfilename = argv[current_idx + 1];
  if (outfilename[0] == '-') {
    printf("\n expected output filename");
    exit(0);
  }
  args->output_file = outfilename;
  return (ArgumentParseResult){.skip_ahead = 2, .exit = false};
}

typedef struct {
  const string arg_name;
  ArgumentAction action;
} ArgumentCmd;

static const ArgumentCmd argument_map[] = {{"--help", show_help},
                                           {"compile", compile},
                                           {"--no-std", no_std},
                                           {"--std-path", def_std_path}};

#define ARGCMD_MAP_SIZE (sizeof(argument_map) / sizeof(ArgumentCmd))

int main(int argc, char *argv[]) {
  Arguments arguments = {.input_files = malloc(512 * sizeof(string)),
                         .output_file = "out.c",
                         .std_path = "./std.kr",
                         .no_std = false};

  printf("\n\n arguments count ? %d \n\n", argc);
  int i = 1;
  // for (int i = 1; i < argc; i++) {
  while (i < argc) {
    int ai = 0;
    bool matched = false;
    for (int ai = 0; ai < ARGCMD_MAP_SIZE; ai++) {
      if (strcmp(argument_map[ai].arg_name, argv[i]) == 0) {
        matched = true;
        ArgumentParseResult result =
            argument_map[ai].action(&arguments, argv, i + 1);

        i += result.skip_ahead;

        if (result.exit) {
          exit(1);
        }
      }
    }

    if (!matched) {
      printf("\nunrecognised argument '%s'\n", argv[i]);
      exit(1);
    }

    i++;
  }

  if (arguments.no_std) {
    compile_file_no_std(arguments.input_files[0], arguments.output_file);
    // compile_program_with_std(BlockStatement *program, BlockStatement *std,
    // char *filename)
  } else {
    compile_file(arguments.input_files[0], arguments.std_path,
                 arguments.output_file);
  }

  return 0;

  // Check if the only argument is "--help" or no arguments are passed
  // if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
  //   printf("usage:\n"
  //          "    krama run ./x.kr                        --execute a file\n"
  //          "    krama compile ./x.kr                    --execute a file\n"
  //          "    krama --help                            --show this\n");
  //   return 0;
  // }

  // // Check for proper usage of "krama run" with an argument
  // if (argc == 3 && strcmp(argv[1], "run") == 0) {
  //   if (argv[2][0] == '\"' || argv[2][0] == '\'') {
  //     // Assuming the inline string is passed with quotes, remove them
  //     // (simplified handling)
  //     char *inlineStr = argv[2] + 1;
  //     inlineStr[strlen(inlineStr) - 1] = '\0';
  //     // run_inline_string(inlineStr);
  //   } else {
  //     run_file(argv[2]);
  //   }
  // } else if (argc == 4 && strcmp(argv[1], "compile") == 0) {
  //   compile_file(argv[2], argv[3]);
  // } else {
  //   printf("Invalid usage. Use 'krama --help' for more information.\n");
  // }

  // return 0;
}
