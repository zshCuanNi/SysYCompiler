#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <cstring>

#include "front/symtab.hpp"
#include "front/parser.tab.hpp"
#include "back/parser_ic.hpp"

extern FILE *yyin;
int yyparse(void);
int yylex(void);

SymbolTable symTab;

FILE *eeyoreOut;
FILE *tiggerOut;
FILE *riscvOut;
FILE *logOut;

using std::string;

int main (int argc, char **argv) {
    int ch;
    
    assert(strcmp("-S", argv[1]) == 0);

    string outFile;
    string log, eeyore, tigger, riscv;

    if (argv[2][0] != '-') { // "./compiler -S testcase.c -o testcase.S"
        yyin = fopen(argv[2], "r");
        outFile = string(argv[4]);
        log = eeyore = tigger = riscv = outFile.substr(0,outFile.find("."));

        log = log.append(".log");
        eeyore = eeyore.append(".eeyore");
        tigger = tigger.append(".tigger");
        riscv = outFile;
    } else if (strcmp("-e", argv[2])  == 0) { // "./compiler -S -e testcase.c -o testcase.S"
        yyin = fopen(argv[3], "r");
        outFile = string(argv[5]);
        log = eeyore = tigger = riscv = outFile.substr(0,outFile.find("."));

        log = log.append(".log");
        eeyore = outFile;
        tigger = tigger.append(".tigger");
        riscv = riscv.append(".riscv");
    } else if (strcmp("-t", argv[2]) == 0) { // "./compiler -S -t testcase.c -o testcase.S"
        yyin = fopen(argv[3], "r");
        outFile = string(argv[5]);
        log = eeyore = tigger = riscv = outFile.substr(0,outFile.find("."));

        log = log.append(".log");
        eeyore = eeyore.append(".eeyore");
        tigger = outFile;
        riscv = riscv.append(".riscv");
    }

    
    
    eeyoreOut = fopen(eeyore.c_str(), "w");
    tiggerOut = fopen(tigger.c_str(), "w");
    riscvOut = fopen(riscv.c_str(), "w");
    logOut = fopen(log.c_str(), "a");

    assert(yyin);
    assert(eeyoreOut);
    assert(tiggerOut);
    assert(riscvOut);
    assert(logOut);

    ASTRoot = new Node();
    // translate SysY to Eeyore
    do {
        yyparse();
    } while (!feof(yyin));

    fprintf(eeyoreOut, "%s", ASTRoot->code.c_str());

    // translate Eeyore to Tigger and/or RiscV
    ParserIC parser = ParserIC(ASTRoot->code, true);

    fprintf(tiggerOut, "%s", parser.codeTigger.c_str());
    fprintf(riscvOut, "%s", parser.codeRiscV.c_str());

    fclose(yyin);
    fclose(eeyoreOut);
    fclose(tiggerOut);
    fclose(riscvOut);
    fclose(logOut);

    return 0;
}