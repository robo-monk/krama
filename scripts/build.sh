set -xe
# time clang src/main.c src/compiler/inference.c src/compiler/CCompiler.c src/compiler/compiler.c src/interpreter/Interpreter.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/KeywordToken.c src/frontend/parser.c src/frontend/tokeniser.c src/frontend/LiteralType.c src/frontend/Op.c src/utils.c -o bin/krama -fsanitize=undefined -Wall
rm -rf bin
mkdir bin
time clang src/main.c src/compiler/inference.c src/compiler/CCompiler.c src/compiler/compiler.c src/interpreter/Interpreter.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/KeywordToken.c src/frontend/parser.c src/frontend/tokeniser.c src/frontend/LiteralType.c src/frontend/Op.c src/utils.c -o bin/krama -fsanitize=undefined -Wall

# time clang src/main.c src/compiler/compiler.c src/hashmap/hashmap.c src/ast/AbstractSyntaxTreeEvaluator.c src/ast/AbstractSyntaxTree.c src/frontend/parser.c src/frontend/tokeniser.c src/frontend/LiteralType.c src/frontend/op.c src/utils.c -o bin/krama
