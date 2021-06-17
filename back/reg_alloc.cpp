#include "parser_ic.hpp"

using std::to_string;

void ParserIC::compile() {
    EntryVarEE *tv;
    EntryFuncEE *tf;
    
    int tmp;
    // global var declaration
    tmp = 0;
    for (auto &i : symTabEE.funcTable["g"]->tmpVars) {
        tv = i.second;
        tv->ifGlobal = true;
        tv->nameTigger = string("v" + to_string(tmp));
        tv->addr = tmp++;
        if (tv->ifArray)
            codeTigger += tv->nameTigger + " = malloc " + to_string(tv->width) + "\n";
        else 
            codeTigger += tv->nameTigger + " = 0\n";
    }

    // function block
    for (string &i : symTabEE.funcNames) {
        runtimeEnv = RuntimeEnv();   // use a new runtime environment for every function when parsing
        int stackSize = 0;

        tf = symTabEE.funcTable[i];
        entryCurFunc = tf;
        // compute stack allocation
        // // callee saved registers
        // stackSize += 12;
        // for (int i = 0; i < 12; i++)
        //     runtimeEnv.stack.push_back(StackElement(i));
        // function parameters
        stackSize += tf->numParams;
        for (int i = 0; i < tf->numParams; i++) {
            // runtimeEnv.stack.push_back(StackElement(12+i, tf->tmpVars["p" + to_string(i)]));
            // tf->tmpVars["p" + to_string(i)].second->addr = 12 + i;
            runtimeEnv.stack.push_back(StackElement(i, tf->tmpVars["p" + to_string(i)]));
            tf->tmpVars["p" + to_string(i)]->addr = i;
        }
        // temp variables in function block
        tmp = 0;
        for (auto &j : tf->tmpVars) {
            if (j.first[0] != 'p')  // if not a parameter
            {
                stackSize += j.second->width / SIZE_OF_INT;
                // j.second->addr = 12 + tf->numParams + tmp;
                j.second->addr = tf->numParams + tmp;
                // here we set ifUpdated to be true if 
                runtimeEnv.stack.push_back(StackElement(j.second->addr, j.second, j.second->ifArray));
                tmp += j.second->width / SIZE_OF_INT;
            }
        }

        // generate function def CODE
        codeTigger += tf->name + " [" + to_string(tf->numParams) + "] [" + to_string(stackSize) + "]\n";

        // // update return registers
        // for (int i = 0; i < tf->numParams; i++)
        //     runtimeEnv.useReg(regReturnNo[i], "p" + to_string(i));

        // store parameters into stack
        for (int i = 0; i < tf->numParams; i++) {
            codeTigger += "\tstore " + regReturn[i] + " " + to_string(i) + "\n";
        }

        for (auto &i : tf->stmtSeq)
            compileStmt(i.second);
        
        codeTigger += "end " + tf->name + "\n";
    }
}

void ParserIC::compileStmt(EENode *stmt) {
    // // entering a basic block
    // if (stmt->ifEntry) {
    //     // store updated variables in stack for previous block
    //     storeAll();
        
    //     // if the entry is not attached with a label
    //     // control will only flow from the exit of the previous block to here
    //     // therefore, there is no need to refresh register file states
    //     if (stmt->label != -1) {
    //         runtimeEnv.refresh();
    //         for (auto &i : tf->tmpVars)
    //             i.second->regs.clear();
    //         // generate label CODE
    //         codeTigger += "l" + to_string(stmt->label) + ":\n";
    //     }
    // }

    // // generate code
    // switch (stmt->lineType) {
    //     case ltCALL:
    //         stmt = (EENodeCall *)stmt;
    //         genCodeCall(stmt);
    //         runTimeEnv.refresh();
    //         break;
    //     case ltRETURN:
    //         stmt = (EENodeReturn *)stmt;
    //         // store return value in a0
    //         if (stmt->dstRet != nullptr)
    //             storeIntoReg(regReturnNo[0], stmt->dstRet);
    //         // // restore callee saved registers
    //         // for (int i = 0; i < 12; i++)
    //         //     if (runTimeEnv.stack[i].ifUpdated)
    //         //         codeTigger += "\tload " + to_string(i) + " " + regCallee[i] + "\n";
    //         codeTigger += "\treturn\n";
    //         break;
    //     case ltGOTO:
    //         stmt = (EENodeGoto *)stmt;
    //         // store updated variables in stack because we reaches exit of block
    //         storeAll();
    //         // pure goto
    //         if (stmt->condition == nullptr)
    //             codeTigger += "\tgoto l" + stmt->dstLabel + "\n";
    //         else {  // if-goto
    //             RegType lr = getRegister(stmt->left);
    //             RegType rr = getRegister(stmt->right);
    //             codeTigger += "\tif " + regName[lr] + " " + stmt->condition->opType + " " + regName[rr] + "goto l" + stmt->dstLabel + "\n";
    //         }
    //         break;
    //     case ltASSIGN:
    //         stmt = (EENodeAssign *)stmt;
    //         genCodeAssign(stmt->left, stmt->right);
    //         break;
    //     default:
    //         break;
    // }

    // label generation
    if (stmt->label != -1)
        codeTigger += "l" + to_string(stmt->label) + ":\n";
    
    switch (stmt->lineType) {
        case ltCALL:
        {
            EENodeCall *stmtTmp = (EENodeCall *)stmt;
            genCodeCall(stmtTmp);
            break;
        }
        case ltRETURN:
        {
            EENodeReturn *stmtTmp = (EENodeReturn *)stmt;
            // store return value in a0
            if (stmtTmp->dstRet != nullptr)
                storeIntoReg(regCallerNo[0]+1, stmtTmp->dstRet);
            codeTigger += "\t" + regReturn[0] + " = " + regCaller[1] + "\n";
            codeTigger += "\treturn\n";
            break;
        }
        case ltGOTO:
        {
            EENodeGoto *stmtTmp = (EENodeGoto *)stmt;
            // pure goto
            if (stmtTmp->condition == nullptr)
                codeTigger += "\tgoto l" + to_string(stmtTmp->dstLabel) + "\n";
            else {  // if-goto
                genCodeAssign(regCallerNo[1], nullptr, stmtTmp->condition);
                codeTigger += "\tif " + regCaller[1] + " != x0 goto l" + to_string(stmtTmp->dstLabel) + "\n";
            }
            break;
        }
        case ltASSIGN:
        {
            EENodeAssign *stmtTmp = (EENodeAssign *)stmt;
            genCodeAssign(0, stmtTmp->left, stmtTmp->right);
            break;
        }
        default:
            break;
    }
}

void ParserIC::storeAll() {
    // for (StackElement &i : runtimeEnv.stack)
    //     // if (i.offset >= 12 && !i.ifUpdated) {   // note that don't reach the elemetns which store original value of callee saved registers
    //     if (!i.ifUpdated) {
    //         // generate store all CODE
    //         codeTigger += "\tstore " + i.var->regs[0].name + " " + i.offset + "\n";
    //         i.ifUpdated = true;
    //     }
}

// generate tigger code in shape "reg = NUM" OR "reg = reg[NUM]"
// directly from eeyore code such as "param NUM OR SYMBOL" OR "return NUM OR SYMBOL"
void ParserIC::storeIntoReg(int regType,
                            EENodeRight *val) {
    string reg = regName[regType];
    // NUM
    if (val->lineType == ltNUM)
        codeTigger += "\t" + reg + " = " + to_string(((EENodeNum *)val)->value) + "\n";
    // SYMBOL OR SYMBOL[NUM]
    else {
        EENodeSymbol *valTmp = (EENodeSymbol *)val;
        EntryVarEE *tv;

        // printf("%s\n", valTmp->name.c_str());
        
        // for global variables
        if (symTabEE.funcTable["g"]->tmpVars.count(valTmp->name)) {
            tv = symTabEE.funcTable["g"]->tmpVars[valTmp->name];
            if (tv->ifArray) {
                codeTigger += "\tloadaddr " + tv->nameTigger + " " + reg + "\n";
            }
            else
                codeTigger += "\tload " + tv->nameTigger + " " + reg + "\n";
        }
        // for lacal variables
        else {
            EntryVarEE *tv = entryCurFunc->tmpVars[valTmp->name];
            if (tv->ifArray)
                codeTigger += "\tloadaddr " + to_string(tv->addr) + " " + reg + "\n";
            else 
                codeTigger += "\tload " + to_string(tv->addr) + " " + reg + "\n";
        }
    }
}

void ParserIC::genCodeCall(EENodeCall *stmt) {
    // int tmp = 0;
    // // save registers, i.e. update all stack elements
    // storeAll();
    // for (int i = 0; i < stmt->params.size(); i++)
    //     // !!!!!!param SYMBOL NUM!!!!!!
    //     storeIntoReg(regReturnNo[i], stmt->params[i]);
    // codeTigger += "\tcall " + stmt->name + "\n";

    for (int i = 0; i < stmt->params.size(); i++)
        genCodeAssign(20 + i, nullptr, stmt->params[i]);
    codeTigger += "\tcall " + stmt->name + "\n";
}

void ParserIC::genCodeAssign(int regType,
                             EENodeRight *left,
                             EENodeRight *right) {
    // RegType dstReg, srcReg;
    // if (left->lineType == ltARRAY) {
    //     dstReg = genCodeArray(left);
    //     srcReg = storeIntoReg(1, right);
    //     codeTigger += "\t" + regName[dstReg] + "[0] = " + regName[srcReg] + "\n";
    //     return;
    // }
    // switch (right->lineType) {
    //     case ltNUM:
            
    // }

    int rtLeft, rtRight;
    if (left != nullptr) {
        if (left->lineType == ltARRAY) {
            genCodeArray(14, 15, (EENodeOp *)left);
            storeIntoReg(15, right);
            codeTigger += "\tt1[0] = " + regName[15] + "\n";
            return;
        }
    }
    if (left == nullptr)
        rtLeft = regType;
    else
        rtLeft = 14;
    
    switch(right->lineType) {
        case ltNUM:
            codeTigger += "\t" + regName[rtLeft] + " = " + to_string(((EENodeNum *)right)->value) + "\n";
            break;
        case ltSYMBOL:
            storeIntoReg(15, right);
            codeTigger += "\t" + regName[rtLeft] + " = " + regName[15] + "\n";
            break;
        case ltARRAY:
            genCodeArray(15, 16, (EENodeOp *)right);
            codeTigger += "\t" + regName[rtLeft] + " = t2[0]\n";
            break;
        case ltCALL:
            genCodeCall((EENodeCall *)right);
            if (rtLeft != 0)
                codeTigger += "\t" + regName[rtLeft] + " = a0\n";
            break;
        case ltUNOP:
        {
            EENodeOp *rightTmp = (EENodeOp *)right;
            storeIntoReg(15, rightTmp->right);
            // codeTigger += "\tt2 = " + rightTmp->opType + "t2\n";
            codeTigger += "\t" + regName[rtLeft] + " = " + rightTmp->opType + "t2\n";
            break;
        }
        case ltBINOP:
        {
            EENodeOp *rightTmp = (EENodeOp *)right;
            storeIntoReg(15, rightTmp->left);
            storeIntoReg(16, rightTmp->right);
            codeTigger += "\t" + regName[rtLeft] + " = t2 " + rightTmp->opType + " t3\n";
            break;
        }
        default:
            break;
    }

    if (left != nullptr)
        storeIntoStack(14, left);
}

void ParserIC::genCodeArray(int rt1,
                            int rt2,
                            EENodeOp *array) {
    EENodeSymbol *symbol = (EENodeSymbol *)array->left;
    string reg1 = regName[rt1], reg2 = regName[rt2];
    // deal with global variable
    if (symTabEE.funcTable["g"]->tmpVars.count(symbol->name)) {
        EntryVarEE *tv = symTabEE.funcTable["g"]->tmpVars[symbol->name];
        codeTigger += "\tloadaddr " + tv->nameTigger + " " + reg1 + "\n";
    }
    else {
        EntryVarEE *tv = entryCurFunc->tmpVars[symbol->name];
        if (tv->ifArray)
            codeTigger += "\tloadaddr " + to_string(tv->addr) + " " + reg1 + "\n";
        else 
            codeTigger += "\tload " + to_string(tv->addr) + " " + reg1 + "\n";
    }

    storeIntoReg(rt2, array->right);
    codeTigger += "\t" + reg1 + " = " + reg1 + " + " + reg2 + "\n";
}

void ParserIC::storeIntoStack(int regType,
                              EENodeRight *val) {
    string reg = regName[regType];
    // number immediately return
    if (val->lineType == ltNUM)
        return;
        
    // variable or array
    EENodeSymbol *valTmp = (EENodeSymbol*)val;
    // global variables
    if (symTabEE.funcTable["g"]->tmpVars.count(valTmp->name)) {
        EntryVarEE *tv = symTabEE.funcTable["g"]->tmpVars[valTmp->name];
        codeTigger += "\tloadaddr " + tv->nameTigger + " t6\n"
                    + "\tt6[0] = " + reg + "\n";
    }
    else
    {        
        codeTigger +="\tstore " + reg + " " + to_string(entryCurFunc->tmpVars[valTmp->name]->addr) + "\n";
    }
    return;
}