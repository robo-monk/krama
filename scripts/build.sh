set -xe
time clang src/main.c src/interpreter/Interpreter.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/parser.c src/frontend/tokeniser.c src/utils.c -o bin/krama
