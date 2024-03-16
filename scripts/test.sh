set -xe

clang -g src/test.c src/interpreter/Interpreter.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/parser.c src/frontend/tokeniser.c src/frontend/LiteralType.c src/frontend/op.c src/utils.c -o bin/test &&
echo 'running tests..' &&
time ./bin/test
