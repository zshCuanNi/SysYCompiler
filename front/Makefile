all: compiler

compiler: tab
	g++ -Wno-register -O2 -lm -std=c++17 parser.tab.cpp lex.yy.cpp ast.cpp symtab.cpp main.cpp -o compiler -Idirs

tab: lex
	bison -d -o parser.tab.cpp parser.y

lex:
	flex -o lex.yy.cpp lexer.l