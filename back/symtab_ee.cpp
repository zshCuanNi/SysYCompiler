#include "symtab_ee.hpp"

EntryVarEE::EntryVarEE(const string &_name,
                       int _width,
                       bool _ifArray)
    : nameEeyore(_name), width(_width), ifArray(_ifArray) {}

EntryFuncEE::EntryFuncEE(const string &_name,
                         int _numParams)
    : name(_name), numParams(_numParams) {}

SymbolTableEE::SymbolTableEE() {
    // "g" represents global context, not in any function blocks
    funcTable.insert(make_pair("g", new EntryFuncEE("g", 0)));
}

void SymbolTableEE::registerVar(const string &name,
                                int width,
                                bool ifArray) {
    funcTable[curFuncName]->tmpVars
        .insert(make_pair(
            name,
            new EntryVarEE(name, width, ifArray)
        ));
}

void SymbolTableEE::registerFunc(const string &name,
                                 int numParams) {
    funcTable.insert(make_pair(name, new EntryFuncEE(name, numParams)));
    funcNames.push_back(name);
}

void SymbolTableEE::addStmt(EENode *stmt) {
    funcTable[curFuncName]->stmtSeq
        .insert(make_pair(stmt->lineno, stmt));
}