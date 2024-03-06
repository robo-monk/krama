#include "frontend/Tokeniser.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils.h"

// produces program that when run
void compile_c_to_str(Token *tokens, string program) {
  int token_index = 0;
  int i = 0;
  while (tokens[token_index].type != PROGRAM_END) {
    Token current_token = tokens[token_index++];

    if (current_token.type == OP) {
      // printf("-- OP %c\n", current_token.value.op_type);
      program[i++] = current_token.value.op_type;
    } else if (current_token.type == NUMBER) {
      char num_str[64];
      sprintf(num_str, "%d!", current_token.value.i32_value);
      int _i = 0;
      while (num_str[_i] != '!') {
        program[i++] = num_str[_i++];
      }
    } else if (current_token.type == OPEN_PAR) {
      program[i++] = '(';
    } else if (current_token.type == CLOSE_PAR) {
      program[i++] = ')';
    } else {
      program[i++] = '?';
    }
  }
}

// prints a json representing the abstract tree
void dbg_abstract_node(AbstractNode *node, int intentation) {
  if (node == NULL) {
    printf("null");
    return;
  }

  printf("{\"token\":\"");
  dbg_token(node->token);
  printf("\",\"lhs\":");
  dbg_abstract_node(node->lhs, intentation + 1);
  printf(",\"rhs\":");
  dbg_abstract_node(node->rhs, intentation + 1);
  printf("}");
}

int main() {
  Token *tokens = malloc(512 * sizeof(Token));
  // tokenise("./expr.meth", tokens);
  tokenise_str("4+390/2\n", tokens);
  dbg_tokens(tokens);

  printf("\n\n");

  string c_program = malloc(4096 * sizeof(char));

  compile_c_to_str(tokens, c_program);

  printf("compiled c program: \n\n");
  printf("%s", c_program);

  printf("\n\n---- ast ----\n\n");
  AbstractNode *node = construct_abstract_node(tokens, 0);
  dbg_abstract_node(node, 0);

  int result = evaluate_abstract_node(node);
  printf("\n\nresult is %d", result);

  // int r = evaluate(tokens);
  // printf("result -> %d", r);

  free(c_program);
  free(tokens);
  printf("\ndone\n");
  return 0;
}
