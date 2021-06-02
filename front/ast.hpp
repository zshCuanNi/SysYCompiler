#ifndef AST_H
#define AST_H

#include <vector>
#include <string>
#include <cstdio>

enum NodeType {
    tINIT,      tNUM,       tARRAY,     tFUNCCALL,  tVAR,
    tPOINTER,   tUNKNOWN,   tCOND
};

enum Operator {
    oADD,       oSUB,       oMUL,       oDIV,       oMOD,
    oGREATER,   oLESS,      oGEQ,       oLEQ,
    oEQ,        oNEQ,
    oPOS,       oNEG,       oNOT,
    oLAND,      oLOR,
    oNONE,      oLOAD
};

enum ReturnType {
    rtINT, rtVOID
};

class Node;

class NodeVarDecl;
class NodeFuncDecl;

class NodeStatement;
class NodeAssignStmt;
class NodeExpressionStmt;
class NodeIfElseStmt;
class NodeWhileStmt;
class NodeBreakStmt;
class NodeContinueStmt;
class NodeReturnStmt;

class NodeExpression;
class NodeArrayExp;
class NodeArithmeticExp;
class NodeConditionExp;
class NodeFunctionCallExp;

using std::string;
using std::vector;

extern Node *ASTRoot;
extern bool conditionFlag;
extern int yylineno;

class Node {
public:
    Node *brother = nullptr;
    Node *son = nullptr;
    string code = "";

    virtual void setBrother(Node *_borther);
    virtual void genCode();
};

class NodeVarDecl : public Node {
public:
    NodeVarDecl *brother = nullptr;
    string name;                        // name of the variable
    bool ifConst;                       // true if it is constant
    bool ifArray = false;               // true if it is an array
    vector<int> width;                  // width of the array for each dimension
    vector<NodeExpression *> value;     // values of the array
    unsigned size;                      // size of the array
    bool ifPointer;                     // true if it is pointer in parameter list
    bool ifParam;                       // true if it is a parameter

    NodeVarDecl(const char *_name,
                NodeExpression *_widthFirstDim,
                NodeExpression *_valueFirst = nullptr,
                bool _ifCosnt = false,
                bool _ifPointer = false,
                bool _ifParam = false
                );
    void setWidth(NodeExpression *_widthFirstDim);
    vector<NodeExpression *> setValue(  vector<int> &_width,
                                        NodeExpression *_valueFirst);
    void genCode();
};

class NodeFuncDecl : public Node {
public:
    vector<NodeVarDecl *> paramList;    // parameter list of the function
    Node *body = nullptr;               // function body, legal if it is Decl or Stmt
    string name;                        // name of the function
    ReturnType returnType;              // type for return value

    NodeFuncDecl(   ReturnType _returnType,
                    const char *_name,
                    Node *_body);
    void addReturn(bool ifAppend);
    void genCode();
};

class NodeStatement : public Node {
public:
    explicit NodeStatement(Node *_son = nullptr);
    virtual void setMember( NodeConditionExp *_condition,
                            Node *_trueStatement,
                            Node *_falseStatement = nullptr);// for if-else and while, set member variables to update information
    void genCode();
};

class NodeAssignStmt : public NodeStatement {
public:
    NodeExpression *leftVar;            // left variable
    NodeExpression *rightExp;           // right expression for assigment

    NodeAssignStmt( NodeExpression *_leftVar,
                    NodeExpression *_rightExp);
    void genCode();
};

class NodeExpressionStmt : public NodeStatement {
public:
    NodeExpression *expression;         // the expression as statement

    explicit NodeExpressionStmt(NodeExpression *_expression);
    void genCode();
};

class NodeIfElseStmt : public NodeStatement {
public:
    NodeConditionExp *condition;
    Node *ifStatement;                  // wondering if there can be NodeStatement
    Node *elseStatement;
    int ifLabel;
    int elseLabel;
    int fallLabel;
    bool ifExistElse;

    NodeIfElseStmt();
    void setMember( NodeConditionExp *_condition,
                    Node *_ifStatement,
                    Node *_elseSatement = nullptr);
    void genCode();
};

class NodeWhileStmt : public NodeStatement {
public:
    NodeConditionExp *condition;
    Node *loopStatement;
    int loopLabel;
    int fallLabel;
    int nextLabel;
    int startLabel;

    NodeWhileStmt();
    void setMember( NodeConditionExp *_condition,
                    Node *_loopStatement,
                    Node *_nextStatement = nullptr);
    void genCode();
};

class NodeBreakStmt : public NodeStatement {
public:
    NodeBreakStmt();
    void genCode();
};

class NodeContinueStmt : public NodeStatement {
public:
    NodeContinueStmt();
    void genCode();
};

class NodeReturnStmt : public NodeStatement {
public:
    ReturnType returnType;
    NodeExpression *returnValue;
    
    NodeReturnStmt(NodeExpression *_returnValue = nullptr);
    void genCode();
};

class NodeExpression : public Node {
public:
    NodeExpression *brother = nullptr;  // array
    NodeExpression *son = nullptr;      // array
    NodeType nodeType;                  // type for expression
    Operator opType;                    // type for operation
    string nameSysY;                    // name in SysY
    string nameEeyore;                  // name in Eeyore
    int value;                          // constant to be folded
    bool ifTemp = false;                // true if it is a temparary expression
    
    NodeExpression( NodeType _nodeType,
                    NodeExpression *_son = nullptr,
                    int _value = 0,
                    Operator _opType = oNONE,
                    const string &_nameSysY = "");
    void setBrother(Node *_brother);    // use Node * for some concern?
    virtual void newTemp();             // what does it mean?
    virtual void constantFold();        // fold constant "int value"
};

class NodeArrayExp : public NodeExpression {
public:
    vector<NodeExpression *> index;
    NodeExpression *expression;         // expression for translation
    string name;                        // name of the array

    NodeArrayExp(   const char *_name,
                    NodeExpression *_indexFirstDim);
    void constantFold();
};

class NodeArithmeticExp : public NodeExpression {
public:
    NodeExpression *lhsExp;             // left hand side expression
    NodeExpression *rhsExp;             // right hand side expression

    NodeArithmeticExp(  Operator _opType,
                        NodeExpression *_lhsExp,
                        NodeExpression *_rhsExp = nullptr);
    void constantFold();
};

class NodeConditionExp : public NodeExpression {
public:
    NodeExpression *lhsExp;
    NodeExpression *rhsExp;
    int trueLabel;
    int falseLabel;

    NodeConditionExp(   Operator _opType,
                        NodeExpression *_lhsExp,
                        NodeExpression *_rhsExp = nullptr);
    void traverse();                    // what does it mean?
};

class NodeFunctionCallExp : public NodeExpression {
public:
    vector<NodeExpression *> paramList; // parameter list of the function
    string name;                        // name of the function

    NodeFunctionCallExp(const char *_name,
                        NodeExpression *_paramFirst);

    void constantFold();
    void newTemp();
};

#endif