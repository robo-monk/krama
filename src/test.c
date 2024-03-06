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

  // printf("    🏎️   | running %s", eval_str);
  if (result != expected) {
    printf("    ❌   |   failed %s   expected %d, got %d\n", eval_str, expected,
           result);
  } else {
    printf("    ✅   | passed %s", eval_str);
  }
}

int main() {

  // simple expressions
  test_eval("4 + 4\n", 8);
  test_eval("4 * 4\n", 16);
  test_eval("4 + 2*2 + 20 / 2 \n", 18);
  test_eval("10 - 2 + 3\n", 11);
  test_eval("8 / 4 * 2\n", 4);
  test_eval("1 + 2 * 3\n", 7);
  test_eval("4 * (2 + 3) - 10 / 2\n", 15);
  test_eval("2 + 2 * 3 - 1 + (4 / 2) * 3\n", 13);

  // varialbles
  test_eval("let a = 5;\n", 5);
  test_eval("let hello = 2\n", 2);
  test_eval("let a = 8-4; a\n", 4);
  test_eval("let a = 2; let b = 3; let c = 5; \n", 5);
  test_eval("let a = 2; let b = 3; let c = 5; a \n", 2);
  test_eval("let a = 3; 2 + a\n", 5);
  test_eval("let a = 4; a + 3\n", 7);
  test_eval("let a = 4; a = 5 + 2;\n", 7);
  test_eval("let a = 4; a = a + 2;\n", 6);
  test_eval("let a = 4; let b = a + 2; b + 5\n", 11);
  return 0;
}
