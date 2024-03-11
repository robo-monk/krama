#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "frontend/Parser.h"
#include "frontend/Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils.h"

void test_eval(string eval_str, int expected) {

  Token *tokens = malloc(512 * sizeof(Token));

  tokenise_str(eval_str, tokens);

  BlockStatement *program = parse_program(tokens);
  ReturnValue ret_val = exec_program(program);

  int result = ret_val.i32_value;

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
  test_eval("let b = 99; let c = (5 + 32 + 2 + b);\n", 138);

  // impl stuff
  test_eval("@impl bing() { 5 }; :bing\n", 5);
  test_eval("@impl hello() { 1 } @impl add1() { 5-:hello }; :add1\n", 4);
  test_eval("@impl hello() { 1;2;3 } :hello\n", 3);
  test_eval("@impl add1(a) { a + 5 }; :add1(1)\n", 6);
  test_eval("let xxx = 42; @impl return_self(a){ a }; :return_self(xxx)\n", 42);
  test_eval("@impl add25(a){a+25}; @impl add50(b){b+50}; "
            ":add50(:add25(5))\n",
            80);

  // scope
  test_eval("@impl add25(a){a+25}; @impl add50(b){b+50}; "
            ":add50(:add25(5)+:add25(25))\n",
            130);

  return 0;
}
