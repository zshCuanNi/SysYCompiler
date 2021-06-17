#include <stdio.h>
#include <assert.h>

#include "front/symtab.hpp"
#include "front/parser.tab.hpp"
#include "back/parser_ic.hpp"

extern FILE *yyin;
int yyparse(void);
int yylex(void);

SymbolTable symTab;

FILE *eeyoreOut;
FILE *tiggerOut;
FILE *logOut;

using std::string;

int main (int argc, char **argv) {

    yyin = fopen(argv[3], "r");
    
    string outFile(argv[5]);
    string log, eeyore, tigger;
    log = outFile.substr(0,outFile.find("."));
    eeyore = outFile.substr(0, outFile.find("."));
    tigger = outFile.substr(0, outFile.find("."));
    log = log.append(".log");
    eeyore = eeyore.append(".eeyore");
    // tigger = tigger.append(".tigger");
    tigger = outFile;
    
    eeyoreOut = fopen(eeyore.c_str(), "w");
    tiggerOut = fopen(tigger.c_str(), "w");
    logOut = fopen(log.c_str(), "a");

    assert(yyin);
    assert(eeyoreOut);
    assert(tiggerOut);
    assert(logOut);

    ASTRoot = new Node();
    // translate SysY to Eeyore
    do {
        yyparse();
    } while (!feof(yyin));

    if (ASTRoot->code.empty()) printf("eww\n");
    fprintf(eeyoreOut, "%s", ASTRoot->code.c_str());

    // translate Eeyore to Tigger and/or RiscV
    ParserIC parser = ParserIC(ASTRoot->code, true, false);

    fprintf(tiggerOut, "%s", parser.codeTigger.c_str());

    fclose(yyin);
    fclose(eeyoreOut);
    fclose(tiggerOut);
    fclose(logOut);

    return 0;
}