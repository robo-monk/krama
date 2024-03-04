#include "stdlib.h"
#include "stdio.h"
#include "utils.h"
#include "tokeniser.h"
#include "ast.h"

void test_eval(string eval_str, int expected) {
    Token *tokens = malloc(512 * sizeof(Token));
    tokenise_str(eval_str, tokens);

    AbstractNode *node = construct_abstract_node(tokens, 0);
    int result = evaluate_abstract_node(node);

    if (result != expected) {
        printf("[x] failed %s   expected %d, got %d", eval_str, expected, result);
    } else {
        printf("[^] passed %s", eval_str);
    }
}

int main() {
    test_eval("4 + 4\n", 8);
    test_eval("4 * 4\n", 16);
    test_eval("4 + 2*2 + 20/2\n", 18);
    test_eval("10 - 2 + 3\n", 11);
    test_eval("8 / 4 * 2\n", 4);
    test_eval("1 + 2 * 3\n", 7);

    return 0;
}
