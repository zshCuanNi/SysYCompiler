#include <stdio.h>
#include <assert.h>

#include "symtab.hpp"
#include "parser.tab.hpp"

extern FILE *yyin;
int yyparse(void);
int yylex(void);
SymbolTable symTab;

using std::string;

int main (int argc, char **argv) {
    FILE *eeyoreOut;

    yyin = fopen(argv[1], "r");
    
    eeyoreOut = fopen(argv[2], "w+");

    assert(yyin);
    assert(eeyoreOut);

    ASTRoot = new Node();
    // translate SysY to Eeyore
    do {
        yyparse();
    } while (!feof(yyin));

    fprintf(eeyoreOut, "%s", ASTRoot->code.c_str());

    return 0;
}