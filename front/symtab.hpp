#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <vector>
#include <unordered_map>
#include "ast.hpp"

#define BLOCK_GLOBAL    0

class EntryVarSymbolTable;
class EntryFuncSymbolTable;
class SymbolTable;

using std::string;
using std::vector;
using std::unordered_map;
using varTable = unordered_map<string, EntryVarSymbolTable>;

extern SymbolTable symTab;

class EntryVarSymbolTable {
public:
    string nameSysY;
    string nameEeyore;
    bool ifConst;                       // true if it is constant
    bool ifArray;                       // true if it is an array
    vector<int> width;                  // width of the array for each dimension
    vector<int> value;                  // values of the array
    bool ifPointer;                     // true if it is pointer in parameter list
    bool ifParam;

    EntryVarSymbolTable() = default;
    EntryVarSymbolTable(const string &_nameSysY,
                        bool _ifConst,
                        bool _ifArray,
                        bool _ifParam,
                        const vector<int> &_width = vector<int>(),
                        const vector<int> &_value = vector<int>());
};

class EntryFuncSymbolTable {
public:
    string nameSysY;
    string nameEeyore;
    vector<EntryVarSymbolTable> paramList;
    vector<EntryVarSymbolTable> symTabLocal;
    ReturnType retType;

    EntryFuncSymbolTable() = default;
    EntryFuncSymbolTable(const string &_name,
                         ReturnType _retType);
};

class SymbolTable {
public:
    vector<varTable> blockStack;
    unordered_map<string, EntryFuncSymbolTable> funcTable;
    string curFuncName = "";
    int blockId = 0;
    bool create = 1;
    int curVarId = 0;
    int curParamId = 0;

    SymbolTable();
    void registerVar(const string &name,
                     bool ifConst = false,
                     bool ifArray = false,
                     bool ifParam = false,
                     const vector<int> &width = vector<int>(),
                     const vector<int> &value = vector<int>());
    bool findVar(const string &name,
                 EntryVarSymbolTable *retEntry = nullptr);

    void registerFunc(const char *name,
                    ReturnType retType);
    bool findFunc(const string &name,
                  EntryFuncSymbolTable *retEntry = nullptr);

    void blockInc();
    void blockDec();
};

#endif