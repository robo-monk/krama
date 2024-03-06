#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "frontend/Parser.h"
#include "frontend/Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils.h"

void test_eval(string eval_str, int expected) {

  Token *tokens = malloc(512 * sizeof(Token));

  tokenise_str(eval_str, tokens);

  Program *program = parse_program(tokens);
  ReturnValue ret_val = exec_program(program);

  int result = ret_val.i32_value;
  if (result != expected) {
    printf("[x] failed %s   expected %d, got %d\n", eval_str, expected, result);
  } else {
    printf("[^] passed %s", eval_str);
  }
}

int main() {
  test_eval("let a = 8; a\n", 8);
  // test_eval("let a = 5;\n", 8);
  // test_eval("4 + 4\n", 8);
  // test_eval("4 * 4\n", 16);
  // test_eval("4 + 2*2 + 20 / 2 \n", 18);
  // test_eval("10 - 2 + 3\n", 11);
  // test_eval("8 / 4 * 2\n", 4);
  // test_eval("1 + 2 * 3\n", 7);
  // test_eval("4 * (2 + 3) - 10 / 2\n", 15);
  // test_eval("2 + 2 * 3 - 1 + (4 / 2) * 3\n", 13);

  return 0;
}
