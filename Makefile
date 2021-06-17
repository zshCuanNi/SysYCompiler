all: compiler

compiler: tab
	g++ -g -Wno-register -O2 -lm -std=c++17 front/parser.tab.cpp front/lex.yy.cpp front/ast.cpp front/symtab.cpp back/ast_ee.cpp back/symtab_ee.cpp back/runtime.cpp back/parser_ic.cpp back/reg_alloc.cpp main.cpp -o compiler -Idirs

tab: lex
	bison -d -o front/parser.tab.cpp front/parser.y

lex:
	flex -o front/lex.yy.cpp front/lexer.l