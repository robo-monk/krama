#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "frontend/Parser.h"
#include "frontend/Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils.h"

void test_eval(string eval_str, int expected) {

  // Token *tokens = malloc(512 * sizeof(Token));

  Tokeniser *t = tokenise_str(eval_str);
  BlockStatement *program = program_from_tokeniser(t);
  ReturnValue ret_val = exec_program(program);

  int result = ret_val.i32_value;

  if (result != expected) {
    printf("    ❌   |   failed %s   expected %d, got %d\n", eval_str, expected,
           result);
  } else {
    printf("    ✅   | passed %s", eval_str);
  }

  free_tokeniser(t);
  free(program);
}

int main() {
  test_eval("if(3 > 2) {5}\n", 5);

  // simple expressions
  test_eval("4 + 4\n", 8);
  test_eval("4 * 4\n", 16);
  test_eval("4 + 2*2 + 20 / 2 \n", 18);
  test_eval("10 - 2 + 3\n", 11);
  test_eval("(8 / 4) * 2\n", 4);
  test_eval("1 + 2 * 3\n", 7);
  test_eval("4 * (2 + 3) - 10 / 2\n", 15);
  test_eval("2 + 2 * 3 - 1 + (4 / 2) * 3\n", 13);

  // varialbles
  test_eval("let bing = 5;\n", 5);
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
  test_eval("fn bing() { 5 }; :bing\n", 5);
  test_eval("fn hello() { 1 } fn add1() { 5-:hello }; :add1\n", 4);
  test_eval("fn hello() { 1;2;3 } :hello\n", 3);
  test_eval("fn add1(a) { a + 5 }; :add1(1)\n", 6);
  // return 0;

  test_eval("let xxx = 42; fn return_self(a){ a }; :return_self(xxx)\n", 42);
  test_eval("fn add25(a){a+25}; fn add50(b){b+50}; "
            ":add50(:add25(5))\n",
            80);

  // scope
  test_eval("fn add25(a){a+25}; fn add50(b){b+50}; "
            ":add50(:add25(5)+:add25(25))\n",
            130);

  // if
  test_eval("if (2 < 3) { 8 } \n", 8);
  // nested if
  test_eval("let b = 5; if (2 < 3) { if (b > 0) {99}} \n", 99);
  test_eval(" if (0 > 0) {99} else {5} \n", 5);

  test_eval("fn factorial(a) {"
            "if (a < 1) {"
            "1"
            "} else {"
            "a*:factorial(a-1)"
            "}"
            "}"
            ":factorial(10)\n",
            3628800);

  test_eval("fn fib(a) { if (a < 2) { a } else { :fib(a-1) + :fib(a-2) }}; "
            ":fib(19)\n",
            4181);

  return 0;
}
