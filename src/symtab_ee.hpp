#ifndef SYMTAB_EE_H
#define SYMTAB_EE_H

#include "ast_ee.hpp"
#include <unordered_map>
#include <map>


using std::string;
using std::map;
using std::unordered_map;
using std::make_pair;

class EntryVarEE {
public:
    string nameEeyore;
    string nameTigger;  // has a tigger name if it is a global variable in tigger
    int width;
    bool ifArray;
    bool ifGlobal = false;
    int addr;           // addr = count in stack
    vector<int> regs;   // all registers storing the variable
                        // note that if the vairavle is an array, regs only store its first address

    EntryVarEE(const string &_name,
               int _width = SIZE_OF_INT,
               bool _ifArray = false);
};

class EntryFuncEE {
public:
    string name;
    int numParams;
    int stackBase; // addr of the stack bottom
    unordered_map<int, EENode *> labels;
    map<int, EENode *> stmtSeq;     // every element is a pair<lineno, statement>
    unordered_map<string, EntryVarEE *> tmpVars;
    // map<int, int> blocks;           // <lineno, lineno> represents lieno of entry and exit for a basic block

    EntryFuncEE(const string &_name,
                int _numParams);
};

class SymbolTableEE {
public:
    string curFuncName = "g";
    unordered_map<string, EntryFuncEE *> funcTable;
    vector<string> funcNames;

    SymbolTableEE();
    void registerVar(const string &name,
                     int width = SIZE_OF_INT,
                     bool ifArray = false);
    void registerFunc(const string &name,
                      int numParams);
    void addStmt(EENode *stmt);
};

#endif