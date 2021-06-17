#ifndef AST_EE_H
#define AST_EE_H

#include "../front/ast.hpp"
#include <vector>
#include <string>

class EENode;
class EENodeAssign;
class EENodeGoto;
class EENodeReturn;
class EENodeRight;
class EENodeSymbol;
class EENodeNum;
class EENodeOp;
class EENodeCall;

enum LineType {
    ltASSIGN,
    ltGOTO,
    ltRETURN,
    ltSYMBOL,
    ltNUM,
    ltBINOP,
    ltUNOP,
    ltARRAY,
    ltCALL
};

using std::string;
using std::vector;

class EENode {
public:
    int lineno;
    int label = -1;     // set if the node is destination of some goto statement
    bool ifEntry = false;   // set if the node is entry of a basic block
    LineType lineType;
    string line;

    EENode(LineType _lineType);
};

class EENodeAssign : public EENode {
public:
    EENodeRight *left;
    EENodeRight *right;

    EENodeAssign(EENodeRight *_left,
                 EENodeRight *_right);
};

class EENodeGoto : public EENode {
public:
    int dstLabel;
    EENodeOp *condition;    // nullptr means pure goto, otherwise means if cond goto label

    EENodeGoto(int _dstLabel,
               EENodeOp *_condition = nullptr);
};

class EENodeReturn : public EENode {
public:
    EENodeRight *dstRet;      // the variable that gets the return value, nullptr if no return value

    explicit EENodeReturn(EENodeRight *_dstRet = nullptr);
};

class EENodeRight : public EENode {
public:
    // abstract class
    EENodeRight(LineType _lineType);
};

class EENodeSymbol : public EENodeRight {
public:
    string name;
    
    EENodeSymbol(const string &_name);
};

class EENodeNum : public EENodeRight {
public:
    int value;

    EENodeNum(int _value);
};

class EENodeOp : public EENodeRight {
public:
    EENodeRight *left;
    EENodeRight *right;
    string opType;

    EENodeOp(LineType _lineType,
             const string &_opType,
             EENodeRight *_left,
             EENodeRight *_right);
};

class EENodeCall : public EENodeRight {
public:
    vector<EENodeRight *> params;
    string name;

    EENodeCall(const string &_name,
               vector<EENodeRight *> &_params);
};

#endif