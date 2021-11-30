#include "symtab.hpp"
#include "assert.h"
#include <stdio.h>

using std::string;
using std::vector;
using std::to_string;

extern FILE *logOut;

Node *ASTRoot;
bool conditionFlag = false;
int labelId = 0;
int tempId = 0;
int globalFallLabel = 0;
int whileNextLabel = 0;
int whileStartLabel = 0;

inline int newLabel() {
    return labelId++;
}

static string genVarDecl(const EntryVarSymbolTable &entryVar,
                         bool ifIdent) {
    // fprintf(logOut, "%s\n", __FUNCTION__);

    if (entryVar.ifParam)
        return "";

    string identStr = ifIdent ? "\t" : "";
    string sizeStr = "";
    if (entryVar.ifArray) {
        int size = SIZE_OF_INT;
        for (int i : entryVar.width)
            size *= i;
        sizeStr = to_string(size) + " ";
    }
    return identStr + "var " + sizeStr + entryVar.nameEeyore + "\n";
}

void Node::setBrother(Node *_brother) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    brother = _brother;
    if (brother)
        code += brother->code;
}

void Node::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (son)
        code += son->code;
}

NodeVarDecl::NodeVarDecl(
    const char*_name,
    NodeExpression *_widthFirstDim,
    NodeExpression *_valueFirst,
    bool _ifConst,
    bool _ifPointer,
    bool _ifParam)
    : ifConst(_ifConst), ifPointer(_ifPointer), ifParam(_ifParam) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    name = string (_name);
    setWidth(_widthFirstDim);

    ifArray = !width.empty();
    if (_valueFirst)    //initializable
        if (ifArray)
            value = setValue(width, _valueFirst->son);
        else
            value = setValue(width, _valueFirst);

    // register variable in symbol table
    vector<int> valueForSymTab;
    for (NodeExpression *i : value)
        valueForSymTab.push_back(i->value);
    // insert 0 behind
    valueForSymTab.insert(valueForSymTab.end(), size - value.size(), 0);
    symTab.registerVar(name, ifConst, ifArray, ifParam, width, valueForSymTab);

    genCode();
}

void NodeVarDecl::setWidth(NodeExpression *_widthFirstDim) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // if the expression looks like "int var[][w1][w2]..."
    if (ifPointer)
        width.push_back(0);
    
    NodeExpression *curWidth = _widthFirstDim;
    size = 1;
    while (curWidth) {
        curWidth->constantFold();
        size *= (unsigned)curWidth->value;
        width.push_back(curWidth->value);
        curWidth = curWidth->brother;
    }
}

vector<NodeExpression *> NodeVarDecl::setValue(
    vector<int> &_width,
    NodeExpression *_valueFirst,
    int filledSize) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    vector<NodeExpression *> ret;

    if (_width.empty()) {   // single value
        _valueFirst->constantFold();
        _valueFirst->newTemp();
        ret.push_back(_valueFirst);
    } else {                // an array
        bool toFillZero = (filledSize == 0);
        int toFillSize = 1;
        for (int i : _width) toFillSize *= i;
        NodeExpression *curNode = _valueFirst;
        // flatten _valueFirst's tree structure
        while (curNode) {
            if (curNode->nodeType == tINIT) {   // not a leaf node
                vector<int> nextWidth = _width;
                nextWidth.erase(nextWidth.begin());
                // get flattened tree from sons
                int sonsToFillSize = toFillSize / _width[0];
                int sonsFilledSize = filledSize % sonsToFillSize;
                vector<NodeExpression *> sonValues = setValue(nextWidth, curNode->son, sonsFilledSize);
                filledSize += sonValues.size();
                for (NodeExpression *i : sonValues)
                    ret.push_back(i);
            } else {                            // a leaf node
                curNode->constantFold();
                curNode->newTemp();
                ret.push_back(curNode);
                filledSize++;
            }
            curNode = curNode->brother;
        }
        if (toFillZero) {
            // expression node of value 0
            NodeExpression *zeroNode = new NodeExpression(tNUM);
            // insert 0 behind
            assert(toFillSize >= filledSize);
            ret.insert(ret.end(), toFillSize - filledSize, zeroNode);
        }
    }

    return ret;
}

void NodeVarDecl::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // no parameter's name will be printed in Eeyore
    if (ifParam)
        return;

    EntryVarSymbolTable entryVar;
    symTab.findVar(name, &entryVar);
    
    if (symTab.blockId == BLOCK_GLOBAL) // declare global variable
        code = genVarDecl(entryVar, false);
    else if (!value.empty()) {          // initialize variable
        if (!ifArray)                   // single value
            code += value[0]->code + "\t" + entryVar.nameEeyore + " = " + value[0]->nameEeyore +"\n";
        else                            // an array
            for (int i = 0; i < value.size(); i++)
                code += value[i]->code + "\t" + entryVar.nameEeyore + "[" + to_string(i * SIZE_OF_INT) + "] = " + value[i]->nameEeyore + "\n"; 
    }
}

NodeFuncDecl::NodeFuncDecl(ReturnType _returnType,
                           const char *_name,
                           Node *_body)
    : body(_body), returnType(_returnType) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    name = string(_name);

    bool ifAppend = (body != nullptr);
    addReturn(ifAppend);

    genCode();
}

void NodeFuncDecl::addReturn(bool ifAppend) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    symTab.curFuncName = name;
    NodeReturnStmt *nodeRet;
    if (symTab.funcTable[name].retType == rtINT)
        nodeRet = new NodeReturnStmt(new NodeExpression(tNUM));
    else
        nodeRet = new NodeReturnStmt();
    
    if (ifAppend) {
        Node *curNode = body;
        // get the last BlockItem, and check whether it is a return statement
        // add a return statement if it isn't
        while (curNode->brother)
            curNode = curNode->brother;
        if (string(typeid(*curNode).name()) != "14NodeReturnStmt")
            body->code += nodeRet->code;
    }
    // deal with the case: void foo() {}, because when {} is reduced to Block, Block = nullptr
    else
        body = nodeRet;
    symTab.curFuncName = "";
}

void NodeFuncDecl::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    EntryFuncSymbolTable entryFunc;
    symTab.findFunc(name, &entryFunc);

    // why we use size of paramList in function entry?
    // because the paramList in func-decl node is even not be modified and keep empty during the whole process!
    code = "f_" + name + " [" + to_string(entryFunc.paramList.size()) + "]\n";
    for (EntryVarSymbolTable i : entryFunc.symTabLocal)
        code += genVarDecl(i, true);
    // initialize global variable
    if (name == "main")
        for (auto i : symTab.blockStack[0]) {
            EntryVarSymbolTable entryVar = i.second;
            if (entryVar.value.empty())
                continue;
            if (entryVar.ifArray)
                for (int i = 0 ; i < entryVar.value.size(); i++)
                    if (entryVar.value[i])
                        code += "\t" + entryVar.nameEeyore + "[" + to_string(i * SIZE_OF_INT) + "] = " + to_string(entryVar.value[i]) + "\n";
                    else
                        code += "";
            else 
                code += "\t" + entryVar.nameEeyore + " = " + to_string(entryVar.value[0]) + "\n";
        }
    
    code += body->code + "end f_" + name + "\n\n";
    tempId = 0;
}

NodeStatement::NodeStatement(Node *_son) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    son = _son;
    genCode();
}

void NodeStatement::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (son)
        code += son->code;
}

void NodeStatement::setMember(NodeConditionExp *_condition,
                              Node *_trueStatement,
                              Node *_falseStatement) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // do nothing
}

NodeAssignStmt::NodeAssignStmt(NodeExpression *_leftVar,
                               NodeExpression *_rightExp)
    : leftVar(_leftVar), rightExp(_rightExp) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    rightExp->constantFold();
    if (leftVar->nodeType == tARRAY)
        rightExp->newTemp();
    
    genCode();
}

void NodeAssignStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // WHY DO CONVERTION MANUALLY?
    code = rightExp->code +
            leftVar->code +
            "\t" + ((NodeArrayExp *)leftVar)->nameEeyore + " = " + rightExp->nameEeyore + "\n";
}

NodeExpressionStmt::NodeExpressionStmt(NodeExpression *_expression)
    : expression(_expression) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    expression->constantFold();

    genCode();
}

void NodeExpressionStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // other type will generate code in other scope
    if (expression->nodeType != tFUNCCALL)
        return;
    NodeFunctionCallExp *funcCallExp = (NodeFunctionCallExp *)expression;
    EntryFuncSymbolTable entryFunc;
    symTab.findFunc(funcCallExp->name, &entryFunc);
    // !!SLIGHTLY DIFFERENT!!
    if (entryFunc.retType == rtINT) {
        funcCallExp->newTemp();
        code = funcCallExp->code;
    } else 
        code = funcCallExp->code + "\t" + funcCallExp->nameEeyore + "\n";
}

NodeIfElseStmt::NodeIfElseStmt() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    fallLabel = globalFallLabel;
    globalFallLabel = newLabel();
}

void NodeIfElseStmt::setMember(NodeConditionExp *_condition,
                               Node *_ifStatement,
                               Node *_elseSatement) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    condition = _condition;
    ifStatement = _ifStatement;
    elseStatement = _elseSatement;
    ifExistElse = (_elseSatement != nullptr);

    condition->falseLabel = elseLabel = newLabel();
    if (ifExistElse)
        condition->trueLabel = ifLabel = newLabel();
    else
        condition->trueLabel = ifLabel = globalFallLabel;
    condition->traverse();

    genCode();
    globalFallLabel = fallLabel;
}

void NodeIfElseStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    code = condition->code;
    if (ifExistElse) {
        code += "l" + to_string(ifLabel) + ":\n" +
                ifStatement->code;
        int tempLabel = newLabel();
        code += "\tgoto l" + to_string(tempLabel) + "\n" +
                "l" + to_string(elseLabel) + ":\n" +
                elseStatement->code +
                "l" + to_string(tempLabel) + ":\n";
    } else
        code += ifStatement->code +
                "l" + to_string(elseLabel) + ":\n";
    globalFallLabel = fallLabel;
}

NodeWhileStmt::NodeWhileStmt() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    startLabel = whileStartLabel;
    nextLabel = whileNextLabel;
    fallLabel = globalFallLabel;
    whileStartLabel = newLabel();
    whileNextLabel = newLabel();
    globalFallLabel = newLabel();
}

void NodeWhileStmt::setMember(NodeConditionExp *_condition,
                              Node *_loopStatement,
                              Node *_nextStatement) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    condition = _condition;
    loopStatement = _loopStatement;
    condition->trueLabel = loopLabel = globalFallLabel;
    condition->falseLabel = whileNextLabel;
    condition->traverse();

    genCode();

    whileStartLabel = startLabel;
    whileNextLabel = nextLabel;
    globalFallLabel = fallLabel;
}

void NodeWhileStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);

    code = "l" + to_string(whileStartLabel) + ":\n" +
            condition->code +
            "l" + to_string(loopLabel) + ":\n" +
            loopStatement->code +
            "\tgoto l" + to_string(whileStartLabel) + "\n" +
            "l" + to_string(whileNextLabel) + ":\n";
}

NodeBreakStmt::NodeBreakStmt() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    genCode();
}

void NodeBreakStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    code = "\tgoto l" + to_string(whileNextLabel) + "\n";
}

NodeContinueStmt::NodeContinueStmt() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    genCode();
}

void NodeContinueStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    code = "\tgoto l" + to_string(whileStartLabel) + "\n";
}

NodeReturnStmt::NodeReturnStmt(NodeExpression *_returnValue)
    : returnValue(_returnValue) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (_returnValue) {
        returnType = rtINT;
        returnValue->constantFold();
        returnValue->newTemp();
    } else
        returnType = rtVOID;

    genCode();
}

void NodeReturnStmt::genCode() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (returnType == rtINT)
        code = returnValue->code +
                "\treturn " + returnValue->nameEeyore + "\n";
    else
        code += "\treturn\n";
}

NodeExpression::NodeExpression(
    NodeType _nodeType,
    NodeExpression *_son,
    int _value,
    Operator _opType,
    const string &_nameSysY)
    : nodeType(_nodeType), son(_son), value(_value), opType(_opType), nameSysY(_nameSysY) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    constantFold();
}
      
void NodeExpression::setBrother(Node *_brother) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    brother = (NodeExpression *)_brother;
}

void NodeExpression::newTemp() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (ifTemp || nodeType == tNUM || nodeType == tVAR)
        return;

    code += "\tt" + to_string(tempId) + " = " + nameEeyore + + "\n";
    nameSysY = "#t" + to_string(tempId);
    nameEeyore = "t" + to_string(tempId++);
    symTab.registerVar(nameSysY);

    ifTemp = true;
}

void NodeExpression::constantFold() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    switch (nodeType) {
        case tNUM:
            nameEeyore = to_string(value);
            break;
        case tVAR: {
            EntryVarSymbolTable entryVar;
            symTab.findVar(nameSysY, &entryVar);
            nameEeyore = entryVar.nameEeyore;
            if (entryVar.ifConst && entryVar.width.empty()) {
                nodeType = tNUM;
                nameEeyore = to_string(entryVar.value[0]);
            }
            break;
        }
        default:
            break;
    }
    return;
}

NodeArrayExp::NodeArrayExp(const char *_name,
                           NodeExpression *_indexFirstDim)
    : NodeExpression(tARRAY) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    name = string(_name);
    
    EntryVarSymbolTable entryVar;
    symTab.findVar(name, &entryVar);
    if (entryVar.ifArray){
        NodeExpression *curNode = _indexFirstDim;
        while (curNode) {
            curNode->constantFold();
            curNode->newTemp();
            index.push_back(curNode);
            curNode = curNode->brother;
        }
    } else {
        nodeType = tVAR;
        nameEeyore = entryVar.nameEeyore;
    }
    constantFold();
}

/* generate index expression tree structure from "vector index" */
static NodeExpression *indexUnfold(
    const vector<NodeExpression *> &index,
    const EntryVarSymbolTable &entryArray) {
    // fprintf(logOut, "%s\n", __FUNCTION__);
    
    if (index.empty())
        return new NodeExpression(tNUM);
    
    NodeExpression *ret = index[0];
    for (int i = 1; i < index.size(); i++) {
        NodeExpression *scale = new NodeExpression(tNUM, nullptr, entryArray.width[i]);
        ret = new NodeArithmeticExp(
            oADD,
            new NodeArithmeticExp(oMUL, ret, scale),
            index[i]);
    }
    return ret;
}

void NodeArrayExp::constantFold() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (!nameSysY.empty())
        return;
    
    EntryVarSymbolTable entryVar;
    symTab.findVar(name, &entryVar);
    switch (nodeType) {
        case tNUM:
            break;
        case tVAR:
            if (entryVar.ifConst) {
                nodeType= tNUM;
                value = entryVar.value[0];
                nameEeyore = to_string(value);
            } else
                nameEeyore = entryVar.nameEeyore;
            break;
        default: {   // an array
            NodeExpression *arrayName = new NodeExpression(tVAR, nullptr, 0, oNONE, name);
            expression = indexUnfold(index, entryVar);

            if (index.size() == entryVar.width.size()) {    // single value
                expression = 
                    new NodeArithmeticExp(
                        oLOAD,
                        arrayName,
                        new NodeArithmeticExp(
                            oMUL,
                            expression,
                            new NodeExpression(tNUM, nullptr, SIZE_OF_INT)
                        )
                    );
            } else {                                        // pointer
                nodeType = tPOINTER;
                int begin = index.size();
                int end = entryVar.width.size();
                int cover = SIZE_OF_INT;
                for (int i = begin; i < end; i++)
                    cover *= entryVar.width[i];
                
                expression = 
                    new NodeArithmeticExp(
                        oADD,
                        arrayName,
                        new NodeArithmeticExp(
                            oMUL,
                            expression,
                            new NodeExpression(tNUM, nullptr, cover)
                        )
                    );
            }
            expression->constantFold();
            code = expression->code;
            nameEeyore = expression->nameEeyore;
            nameSysY.append("+DONE");
            break;
        }
    }
}

NodeArithmeticExp::NodeArithmeticExp(Operator _opType,
                                     NodeExpression *_lhsExp,
                                     NodeExpression *_rhsExp)
    : NodeExpression(tUNKNOWN, nullptr, 0, _opType), lhsExp(_lhsExp), rhsExp(_rhsExp) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    constantFold();
}

void NodeArithmeticExp::constantFold() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (!nameSysY.empty())
        return;
    
    if (rhsExp) {   // for binary operator
        // two constants can be folded to one when compiling
        if (lhsExp->nodeType == tNUM && rhsExp->nodeType == tNUM) {
            switch (opType) {
                case oADD:
                    value = (lhsExp->value + rhsExp->value);
                    break;
                case oSUB:
                    value = (lhsExp->value - rhsExp->value);
                    break;
                case oMUL:
                    value = (lhsExp->value * rhsExp->value);
                    break;
                case oDIV:
                    value = (lhsExp->value / rhsExp->value);
                    break;
                case oMOD:
                    value = (lhsExp->value % rhsExp->value);
                    break;
                case oGREATER:
                    value = (lhsExp->value > rhsExp->value);
                    break;
                case oLESS:
                    value = (lhsExp->value < rhsExp->value);
                    break;
                case oGEQ:
                    value = (lhsExp->value >= rhsExp->value);
                    break;
                case oLEQ:
                    value = (lhsExp->value <= rhsExp->value);
                    break;
                case oEQ:
                    value = (lhsExp->value == rhsExp->value);
                    break;
                case oNEQ:
                    value = (lhsExp->value != rhsExp->value);
                    break;
                default:
                    break;
            }
            nodeType = tNUM;
            nameEeyore = to_string(value);
        } 
        // if not constant, explicitly generate code for them
        else {
            static const string opStr[] = 
                { "+", "-", "*", "/", "%", 
                ">", "<", ">=", "<=", "==", "!=" };
            
            lhsExp->newTemp();
            rhsExp->newTemp();
            // opType is in opStr
            if ((int)opType >= 0 && (int)opType <= 10)
                nameEeyore = lhsExp->nameEeyore + " " + opStr[(int)opType] + " " + rhsExp->nameEeyore;
            else if (opType == oLOAD)
                nameEeyore = lhsExp->nameEeyore + "[" + rhsExp->nameEeyore + "]";
            code = lhsExp->code + rhsExp->code;
        }
    } else {    // for unary operator
        // canstant can be folded
        if (lhsExp->nodeType == tNUM) {
            nodeType = lhsExp->nodeType;
            switch (opType) {
                // oPOS will not be seen because we directly generate a UnaryExp Node when encountering "+" when parsing
                case oPOS:
                    value = +(lhsExp->value);
                    break;
                case oNEG:
                    value = -(lhsExp->value);
                    break;
                case oNOT:
                    value = !(lhsExp->value);
                    break;
                default:
                    break;
            }
            nameEeyore = to_string(value);
        } 
        // if not constant, explicitly generate code for it
        else {
            lhsExp->newTemp();
            switch (opType) {
                // oPOS will not be seen because we directly generate a UnaryExp Node when encountering "+" when parsing
                case oPOS:
                    nameEeyore = "+" + lhsExp->nameEeyore;
                    break;
                case oNEG:
                    nameEeyore = "-" + lhsExp->nameEeyore;
                    break;
                case oNOT:
                    nameEeyore = "!" + lhsExp->nameEeyore;
                    break;
                default:
                    break;
            }
            code = lhsExp->code;
        }
    }
    nameSysY.append("+DONE");
}

NodeConditionExp::NodeConditionExp(Operator _opType,
                                   NodeExpression *_lhsExp,
                                   NodeExpression *_rhsExp)
    : NodeExpression(tCOND, nullptr, 0, _opType), lhsExp(_lhsExp), rhsExp(_rhsExp) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    // do nothing
}

// traverse the whole tree structure of condition expression and generate code
void NodeConditionExp::traverse() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (rhsExp == nullptr) {    // a leaf node
        lhsExp->constantFold();
        lhsExp->newTemp();
        code += lhsExp->code;
// 2: 0 3 0
// 0: 2 0 0
// 2: 0 1 0
// 0: 2 1 0
// 1
// 0: 7 5 5
// 2: 5 8 5
// 0: 7 5 5
// 2: 5 6 5

        if (trueLabel != globalFallLabel) {
            if (falseLabel == globalFallLabel)
                code += "\tif " + lhsExp->nameEeyore + " == 1 goto l" + to_string(trueLabel) + "\n";
            else
                code += "\tif " + lhsExp->nameEeyore + " == 0 goto l" + to_string(falseLabel) + "\n";
        }
        else if (falseLabel != globalFallLabel) {
            code += "\tif " + lhsExp->nameEeyore + " == 0 goto l" + to_string(falseLabel) + "\n";
        }
    } else {                    // not a leaf node
        NodeConditionExp *lhsCond = (NodeConditionExp *)lhsExp;
        NodeConditionExp *rhsCond = (NodeConditionExp *)rhsExp;
        switch (opType) {
            case oLAND:
                lhsCond->trueLabel = globalFallLabel;
                if (falseLabel == globalFallLabel)
                    lhsCond->falseLabel = newLabel();
                else
                    lhsCond->falseLabel = falseLabel;
                rhsCond->trueLabel = trueLabel;
                rhsCond->falseLabel = falseLabel;
                lhsCond->traverse();
                rhsCond->traverse();

                code += lhsCond->code +
                        rhsCond->code;
                if (falseLabel == globalFallLabel)
                    code += "l" + to_string(lhsCond->falseLabel) + ":\n";
                break;
            case oLOR:
                if (trueLabel == globalFallLabel)
                    lhsCond->trueLabel = newLabel();
                else
                    lhsCond->trueLabel = trueLabel;
                lhsCond->falseLabel = globalFallLabel;
                rhsCond->trueLabel = trueLabel;
                rhsCond->falseLabel = falseLabel;
                lhsCond->traverse();
                rhsCond->traverse();

                code += lhsCond->code +
                        rhsCond->code;
                if (trueLabel == globalFallLabel)
                    code += "l" + to_string(lhsCond->trueLabel) + ":\n";
                break;
            default:
                break;
        }
    }
}

NodeFunctionCallExp::NodeFunctionCallExp(
    const char *_name,
    NodeExpression *_paramFirst)
    : NodeExpression(tFUNCCALL) {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    name = string(_name);

    // deal with macro command for time counting
    if (name == "starttime" || name == "stoptime") {
        name = "_sysy_" + name;
        _paramFirst = new NodeExpression(tNUM, nullptr, yylineno);
    }

    NodeExpression *curParam = _paramFirst;
    while (curParam) {
        paramList.push_back(curParam);
        curParam = curParam->brother;
    }
    for (NodeExpression *i : paramList) {
        i->constantFold();
        i->newTemp();
    }
    constantFold();
}

void NodeFunctionCallExp::constantFold() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (nameSysY.empty()) {
        for (NodeExpression *i : paramList)
            code += i->code;
        for (NodeExpression *i : paramList)
            code += "\tparam " + i->nameEeyore + "\n";
        nameEeyore = "call f_" + name;
        // ??WHY TO DO SO??
        nameSysY.append("+DONE");
    }
}

void NodeFunctionCallExp::newTemp() {
    // fprintf(logOut, "%s::%s\n", typeid(*this).name(), __FUNCTION__);
    
    if (ifTemp || nodeType == tNUM || nodeType == tVAR)
        return;
    
    nameSysY = "#t" + to_string(tempId);
    nameEeyore = "t" + to_string(tempId++);
    code += "\t" + nameEeyore + " = call f_" + name + "\n";
    symTab.registerVar(nameSysY);
    
    ifTemp = true;
}