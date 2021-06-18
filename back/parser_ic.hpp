#ifndef PARSER_IC_H
#define PARSER_IC_H

#include <sstream>
#include <vector>
#include <unordered_map>
#include "runtime.hpp"

using std::string;
using std::vector;
using std::unordered_map;
using std::stringstream;

class ParserIC {
public:
    string codeEeyore;
    string codeTigger;
    string codeRiscV;
    SymbolTableEE symTabEE;
    RuntimeEnv runtimeEnv;
    EntryFuncEE *entryCurFunc;
    int STK;

    unordered_map<int, int> labels;    // <label id, lineno>
    vector<EENodeRight *> params;

    bool ifTigger;
    bool ifRiscV;

    ParserIC(const string &_codeEeyore,
             bool _ifTigger = false,
             bool _ifRiscV = false);
    void parseLine(const string &line,
                      int lineno);
    EENodeRight *parseValue(const string &code);
    EENodeRight *parseRight(const string &input);
    void genVarDeclEE(stringstream &input);

    void compile();
    void compileStmt(EENode *stmt);
    void storeAll(); // dummy function which is empty
    void storeIntoReg(int regType,
                      EENodeRight *val);
    void genCodeCall(EENodeCall *stmt);
    void genCodeAssign(int regType,
                       EENodeRight *left,
                       EENodeRight *right);
    void genCodeArray(int rt1,
                      int rt2,
                      EENodeOp *array);
    void storeIntoStack(int regType,
                        EENodeRight *val);

    string plusIndent(const string &text);
    bool ifOutInt12(int val);
    void genADDI(const string &reg1,
                 const string &reg2,
                 const int val);
    void genSW(const string &reg1,
               const string &reg2,
               const int val);
    void genLW(const string &reg1,
               const string &reg2,
               const int val);
};

#endif