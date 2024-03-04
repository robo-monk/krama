clang test.c statements.c parser.c tokeniser.c utils.c ast.c -o test
echo 'running tests..'
time ./test
