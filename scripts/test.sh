set -xe

clang src/test.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/parser.c src/frontend/tokeniser.c src/utils.c -o bin/test &&
echo 'running tests..' &&
time ./bin/test
