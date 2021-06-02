#include "symtab.hpp"
#include <assert.h>

using std::to_string;

EntryVarSymbolTable::EntryVarSymbolTable(   const string &_nameSysY,
                                            bool _ifConst,
                                            bool _ifArray,
                                            bool _ifParam,
                                            const vector<int> &_width,
                                            const vector<int> &_value)
    : ifConst(_ifConst), ifArray(_ifArray), ifParam(_ifParam), width(_width), value(_value) {
    // do nothing
    // !!NO ASSIGNMENT FOR NAMESYSY AND NAMEEEYORE!!
}

EntryFuncSymbolTable::EntryFuncSymbolTable( const string &_nameSysY,
                                            ReturnType _retType)
    : nameSysY(_nameSysY), retType(_retType) {
    nameEeyore = "f_" + nameSysY;
}

SymbolTable::SymbolTable() {
    blockStack.clear();
    funcTable.clear();
    blockStack.push_back(varTable());

    // register runtime library functions
    const char *funcGetter[] = { "getint", "getch", "getarray" };
    for (const char *name : funcGetter)
        registerFunc(name, rtINT);
    funcTable["getarray"].paramList.push_back(EntryVarSymbolTable(
        "p0", false, true, true, vector<int>(1, 0), vector<int>()
    ));

    const char *funcPutter[] = { "putint", "putch", "putarray" };
    for (const char *name : funcPutter) {
        registerFunc(name, rtVOID);
        funcTable[name].paramList.push_back(EntryVarSymbolTable(
            "", false, false, true, vector<int>(), vector<int>()
        ));
    }
    funcTable["putarray"].paramList.push_back(EntryVarSymbolTable(
        "", false, true, true, vector<int>(1, 0), vector<int>()
    ));

    const char *funcTime[] = { "_sysy_starttime", "_sysy_stoptime" };
    for (const char *name : funcTime) {
        registerFunc(name, rtVOID);
        funcTable[name].paramList.push_back(EntryVarSymbolTable(
            "", false, false, true, vector<int>(), vector<int>()
        ));
    }
}

void SymbolTable::registerVar(  const string &name,
                                bool ifConst,
                                bool ifArray,
                                bool ifParam,
                                const vector<int> &width,
                                const vector<int> &value) {
    EntryVarSymbolTable newEntryVar(name, ifConst, ifArray, ifParam, width, value);

    if (ifParam) {      // it is a parameter
        newEntryVar.nameEeyore = "p" + to_string(curParamId++);
        if (blockId != BLOCK_GLOBAL)        // not in global scope
            funcTable[curFuncName].paramList.push_back(newEntryVar);
    } else 
        if (name[0] == '#')
            newEntryVar.nameEeyore = name.substr(1);
        else
            newEntryVar.nameEeyore = "T" + to_string(curVarId++);
    
    if (blockId != BLOCK_GLOBAL)
        funcTable[curFuncName].symTabLocal.push_back(newEntryVar);
    
    blockStack[blockId][name] = newEntryVar;
}

bool SymbolTable::findVar(  const string &name,
                            EntryVarSymbolTable *retEntry) {
    for (int i = blockId; i >= 0; i--)
        if (blockStack[i].find(name) != blockStack[i].end()) {   // exist in stack
            assert(retEntry != nullptr);
            *retEntry = blockStack[i][name];
            return true;
        }
    return false;
}

void SymbolTable::registerFunc( const char *name,
                                ReturnType retType) {
    curFuncName = string(name);
    funcTable[curFuncName] = EntryFuncSymbolTable(curFuncName, retType);
}

bool SymbolTable::findFunc( const string &name,
                            EntryFuncSymbolTable *retEntry) {
    if (funcTable.find(name) != funcTable.end()) {              // exist in table
        assert(retEntry != nullptr);
        *retEntry = funcTable[name];
        return true;
    }
    return false;
}

void SymbolTable::blockInc() {
    if (create) {
        blockId++;
        blockStack.push_back(varTable());
    }
    create = 1;
}

void SymbolTable::blockDec() {
    blockId--;
    blockStack.pop_back();
    if (blockId == BLOCK_GLOBAL) {      // in global scope, get out of the function
        curParamId = 0;
        curFuncName.clear();
    }
}