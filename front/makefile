out: lexer

lexer: lex.yy.c token.h
	gcc -o $@ $<

lex.yy.c: lexer.l
	flex $<