#include "ast/AbstractSyntaxTreeEvaluator.h"
#include "frontend/parser.h"
#include "frontend/tokeniser.h"
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
  test_eval("def bing() { 5 }; :bing\n", 5);
  test_eval("def hello() { 1 } def add1() { 5-:hello }; :add1\n", 4);
  test_eval("def hello() { 1;2;3 } :hello\n", 3);
  test_eval("def add1(a: i32) { a + 5 }; :add1(1)\n", 6);
  // return 0;

  test_eval("let xxx = 42; def return_self(a: i32){ a }; :return_self(xxx)\n",
            42);
  test_eval("def add25(a:i32){a+25}; def add50(b:i32){b+50}; "
            ":add50(:add25(5))\n",
            80);

  // scope
  test_eval("def add25(a:i32){a+25}; def add50(b:i32){b+50}; "
            ":add50(:add25(5)+:add25(25))\n",
            130);

  // if
  test_eval("if (2 < 3) { 8 } \n", 8);
  // nested if
  test_eval("let b = 5; if (2 < 3) { if (b > 0) {99}} \n", 99);
  test_eval(" if (0 > 0) {99} else {5} \n", 5);

  test_eval("def factorial(a:i32) {"
            "if (a < 1) {"
            "1"
            "} else {"
            "a*:factorial(a-1)"
            "}"
            "}"
            ":factorial(10)\n",
            3628800);

  test_eval(
      "def fib(a:i32) { if (a < 2) { a } else { :fib(a-1) + :fib(a-2) }}; "
      ":fib(19)\n",
      4181);

  test_eval("def add(a: i32, b: i32) {a+b}; :add(3-1, 3+3);\n", 8);
  test_eval("def add(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) "
            "{a+b+c+d+e+f}; :add(5, 5, 5, 5, 5, 5);\n",
            30);
  test_eval("def add(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) "
            "{a+b+c+d+e+f}; :add(1, 2, 3, 4, 5, 6);\n",
            21);
  test_eval("def add(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) "
            "{a+b+c+d+e+f}; let hello = 5; :add(hello, 2, 3, 4, 5, 6);\n",
            25);

  // this shit ||
  // test_eval("def |x: i32| incr() {x+1}; 4:incr \n", 5);
  return 0;
}
