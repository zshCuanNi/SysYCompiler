#include <stdio.h>
#include <assert.h>

#include "symtab.hpp"
#include "parser.tab.hpp"

extern FILE *yyin;
int yyparse(void);
int yylex(void);
SymbolTable symTab;

FILE *eeyoreOut;
FILE *logOut;

using std::string;

int main (int argc, char **argv) {

    yyin = fopen(argv[3], "r");
    
    eeyoreOut = fopen(argv[5], "w");
    string outFile(argv[5]);
    outFile = outFile.substr(0, outFile.find(".eeyore")).append(".log");
    logOut = fopen(outFile.c_str(), "a");

    assert(yyin);
    assert(eeyoreOut);
    assert(logOut);

    ASTRoot = new Node();
    // translate SysY to Eeyore
    do {
        yyparse();
    } while (!feof(yyin));

    fprintf(eeyoreOut, "%s", ASTRoot->code.c_str());

    fclose(yyin);
    fclose(eeyoreOut);
    fclose(logOut);

    return 0;
}