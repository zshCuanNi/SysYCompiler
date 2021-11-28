#include "parser_ic.hpp"

using std::to_string;

string ParserIC::plusIndent(const string &text) {
    int indent;
    if (text.length() <= 9)
        indent = 10 - text.length();
    else indent = 1;
    return text + string(indent, ' ');
}

bool ParserIC::ifOutInt12(int val) {
    if (val < -2048 || val > 2047) {
        codeRiscV += "\t" + plusIndent("li") + "s0, " + to_string(val) + "\n";
        return true;
    }
    return false;
}

void ParserIC::genADDI(const string &reg1, const string &reg2, const int val) {
    ifSW = false;
    if (ifOutInt12(val))
        codeRiscV += "\t" + plusIndent("add") + reg1 + ", " + reg2 + ", s0\n";
    else 
        codeRiscV += "\t" + plusIndent("addi") + reg1 + ", " + reg2 + ", " + to_string(val) + "\n";
}

void ParserIC::genSW(const string &reg1, const string &reg2, const int val) {
    ifSW = true;
    reg1SW = reg1, reg2SW = reg2, valSW = val;
    if (ifOutInt12(val))
        codeRiscV += "\t" + plusIndent("add") + "s0, " + reg2 + ", s0\n" +
                   + "\t" + plusIndent("sw") + reg1 + ", 0(s0)\n";
    else
        codeRiscV += "\t" +plusIndent("sw") + reg1 + ", " + to_string(val) + "(" + reg2 + ")\n";
}

void ParserIC::genLW(const string &reg1, const string &reg2, const int val) {
    if (ifSW && reg1 == reg1SW && reg2 == reg2SW && val == valSW) {
        ifSW = false;
        return; 
    }
    if (ifOutInt12(val))
        codeRiscV += "\t" + plusIndent("add") + "s0, " + reg2 + ", s0\n" +
                   + "\t" + plusIndent("lw") + reg1 + ", 0(s0)\n";
    else {
        codeRiscV += "\t" + plusIndent("lw") + reg1 + ", " + to_string(val) + "(" + reg2 + ")\n";
    }
}

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
        // tigger
        if (tv->ifArray)
            codeTigger += tv->nameTigger + " = malloc " + to_string(tv->width) + "\n";
        else 
            codeTigger += tv->nameTigger + " = 0\n";
        // riscv
        ifSW = false;
        if (tv->ifArray)
            codeRiscV += "\t" + plusIndent(".comm") + tv->nameTigger + ", " + to_string(tv->width) + ", 4\n\n";
        else
            codeRiscV += "\t" + plusIndent(".global") + tv->nameTigger + "\n"
                       + "\t" + plusIndent(".section") + ".sdata\n"
                       + "\t" + plusIndent(".align") + "2\n"
                       + "\t" + plusIndent(".type") + tv->nameTigger + ", @object\n"
                       + "\t" + plusIndent(".size") + tv->nameTigger + ", 4\n"
                       + tv->nameTigger + ":\n"
                       + "\t" + plusIndent(".word") + "0\n\n";
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
        // tigger
        codeTigger += tf->name + " [" + to_string(tf->numParams) + "] [" + to_string(stackSize) + "]\n";
        // riscv
        ifSW = false;
        string rname = tf->name.substr(2);
        STK = (stackSize / 4 + 1) * 16;
        codeRiscV += "\t" + plusIndent(".text") + "\n"
                   + "\t" + plusIndent(".align") + "2\n"
                   + "\t" + plusIndent(".global") + rname + "\n"
                   + "\t" + plusIndent(".type") + rname + ", @function\n"
                   + rname + ":\n";
        genADDI("sp", "sp", -STK);
        genSW("ra", "sp", STK - 4);

        // // update return registers
        // for (int i = 0; i < tf->numParams; i++)
        //     runtimeEnv.useReg(regReturnNo[i], "p" + to_string(i));

        // store parameters into stack
        for (int i = 0; i < tf->numParams; i++) {
            // tigger
            codeTigger += "\tstore " + regReturn[i] + " " + to_string(i) + "\n";
            // riscv
            genSW(regReturn[i], "sp", i * 4);
        }

        for (auto &i : tf->stmtSeq)
            compileStmt(i.second);
        
        // tigger
        codeTigger += "end " + tf->name + "\n";
        // riscv
        ifSW = false;
        codeRiscV += "\t" + plusIndent(".size") + rname + ", .-" + rname + "\n\n";
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
    if (stmt->label != -1) {
        // tigger
        codeTigger += "l" + to_string(stmt->label) + ":\n";
        // riscv
        ifSW = false;
        codeRiscV += ".l" + to_string(stmt->label) + ":\n";
    }

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
            if (stmtTmp->dstRet != nullptr) {
                storeIntoReg(regCallerNo[1], stmtTmp->dstRet);
                // tigger
                codeTigger += "\t" + regReturn[0] + " = " + regCaller[1] + "\n";
                // riscv
                ifSW = false;
                codeRiscV += "\t" + plusIndent("mv") + regReturn[0] + ", " + regCaller[1] + "\n";
            }
            
            // tigger
            codeTigger += "\treturn\n";
            // riscv
            genLW("ra", "sp", STK - 4);
            genADDI("sp", "sp", STK);
            codeRiscV += "\tret\n";
            break;
        }
        case ltGOTO:
        {
            EENodeGoto *stmtTmp = (EENodeGoto *)stmt;
            // pure goto
            if (stmtTmp->condition == nullptr) {
                // tigger
                codeTigger += "\tgoto l" + to_string(stmtTmp->dstLabel) + "\n";
                // riscv
                ifSW = false;
                codeRiscV += "\tj .l" + to_string(stmtTmp->dstLabel) + "\n";
            }
            else {  // if-goto
                genCodeAssign(regCallerNo[1], nullptr, stmtTmp->condition);
                // tigger
                codeTigger += "\tif " + regCaller[1] + " != x0 goto l" + to_string(stmtTmp->dstLabel) + "\n";
                // riscv
                ifSW = false;
                codeRiscV += "\tbne " + regCaller[1] + ", x0, .l" + to_string(stmtTmp->dstLabel) + "\n";
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
    if (val->lineType == ltNUM) {
        // tigger
        codeTigger += "\t" + reg + " = " + to_string(((EENodeNum *)val)->value) + "\n";
        // riscv
        ifSW = false;
        codeRiscV += "\t" + plusIndent("li") + reg + ", " + to_string(((EENodeNum *)val)->value) + "\n";
    }
    // SYMBOL OR SYMBOL[NUM]
    else {
        EENodeSymbol *valTmp = (EENodeSymbol *)val;
        EntryVarEE *tv;

        // printf("%s\n", valTmp->name.c_str());
        
        // for global variables
        if (symTabEE.funcTable["g"]->tmpVars.count(valTmp->name)) {
            tv = symTabEE.funcTable["g"]->tmpVars[valTmp->name];
            if (tv->ifArray) {
                // tigger
                codeTigger += "\tloadaddr " + tv->nameTigger + " " + reg + "\n";
                // riscv
                ifSW = false;
                codeRiscV += "\t" + plusIndent("la") + reg + ", " + tv->nameTigger + "\n";
            }
            else {
                // tigger
                codeTigger += "\tload " + tv->nameTigger + " " + reg + "\n";
                // riscv
                ifSW = false;
                codeRiscV += "\t" + plusIndent("lui") + reg + ", %hi(" + tv->nameTigger + ")\n"
                           + "\t" + plusIndent("lw") + reg + ", %lo(" + tv->nameTigger + ")(" + reg + ")\n";
            }
        }
        // for lacal variables
        else {
            EntryVarEE *tv = entryCurFunc->tmpVars[valTmp->name];
            if (tv->ifArray) {
                // tigger
                codeTigger += "\tloadaddr " + to_string(tv->addr) + " " + reg + "\n";
                // riscv
                genADDI(reg, "sp", tv->addr * 4);
            }
            else {
                // tigger
                codeTigger += "\tload " + to_string(tv->addr) + " " + reg + "\n";
                // riscv
                genLW(reg, "sp", tv->addr * 4);
            }
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
    
    // tigger
    codeTigger += "\tcall " + stmt->name + "\n";
    // riscv
    ifSW = false;
    codeRiscV += "\t" + plusIndent("call") + stmt->name.substr(2) + "\n";
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
            genCodeArray(regCallerNo[1], regCallerNo[2], (EENodeOp *)left);
            storeIntoReg(regCallerNo[2], right);
            
            // tigger
            codeTigger += "\tt1[0] = " + regCaller[2] + "\n";
            // riscv
            genSW(regCaller[2], regCaller[1], 0);
            return;
        }
    }
    if (left == nullptr)
        rtLeft = regType;
    else
        rtLeft = regCallerNo[1];
    
    switch(right->lineType) {
        case ltNUM:
            // tigger
            codeTigger += "\t" + regName[rtLeft] + " = " + to_string(((EENodeNum *)right)->value) + "\n";
            // riscv
            ifSW = false;
            codeRiscV += "\t" + plusIndent("li") + regName[rtLeft] + ", " + to_string(((EENodeNum *)right)->value) + "\n";
            break;
        case ltSYMBOL:
            storeIntoReg(regCallerNo[2], right);

            // tigger
            codeTigger += "\t" + regName[rtLeft] + " = " + regCaller[2] + "\n";
            // riscv
            ifSW = false;
            codeRiscV += "\t" + plusIndent("mv") + regName[rtLeft] + ", " + regCaller[2] + "\n";
            break;
        case ltARRAY:
            genCodeArray(regCallerNo[2], regCallerNo[3], (EENodeOp *)right);

            // tigger
            codeTigger += "\t" + regName[rtLeft] + " = t2[0]\n";
            // riscv
            genLW(regName[rtLeft], regCaller[2], 0);
            break;
        case ltCALL:
            genCodeCall((EENodeCall *)right);
            if (rtLeft != 0) {
                // tigger
                codeTigger += "\t" + regName[rtLeft] + " = a0\n";
                // riscv
                ifSW = false;
                codeRiscV += "\t" + plusIndent("mv") + regName[rtLeft] + ", " + regReturn[0] + "\n";
            }
            break;
        case ltUNOP:
        {
            EENodeOp *rightTmp = (EENodeOp *)right;
            storeIntoReg(15, rightTmp->right);
            
            // tigger
            codeTigger += "\t" + regName[rtLeft] + " = " + rightTmp->opType + "t2\n";
            // riscv
            ifSW = false;
            switch (rightTmp->opType[0]) {
                case '-':
                    codeRiscV += "\t" + plusIndent("neg") + regName[rtLeft] + ", " + regCaller[2] + "\n";
                    break;
                case '!':
                    codeRiscV += "\t" + plusIndent("seqz") + regName[rtLeft] + ", " + regCaller[2] + "\n";
                    break;
                default:
                    break;
            }
            break;
        }
        case ltBINOP:
        {
            EENodeOp *rightTmp = (EENodeOp *)right;
            storeIntoReg(regCallerNo[2], rightTmp->left);
            storeIntoReg(regCallerNo[3], rightTmp->right);

            // tigger
            codeTigger += "\t" + regName[rtLeft] + " = t2 " + rightTmp->opType + " t3\n";
            // riscv
            ifSW = false;
            string reg1, reg2, reg3;
            reg1 = regName[rtLeft], reg2 = regCaller[2], reg3 = regCaller[3];
            string regComb = reg1 + ", " + reg2 + ", " + reg3 + "\n";
            switch (rightTmp->opType[0]) {
                case '+':
                    codeRiscV += "\t" + plusIndent("add") + regComb;
                    break;
                case '-':
                    codeRiscV += "\t" + plusIndent("sub") + regComb;
                    break;
                case '*':
                    codeRiscV += "\t" + plusIndent("mul") + regComb;
                    break;
                case '/':
                    codeRiscV += "\t" + plusIndent("div") + regComb;
                    break;
                case '%':
                    codeRiscV += "\t" + plusIndent("rem") + regComb;
                    break;
                case '<':
                    if (rightTmp->opType.length() == 1) // "<"
                        codeRiscV += "\t" + plusIndent("slt") + regComb;
                    else // "<="
                        codeRiscV += "\t" + plusIndent("sgt") + regComb
                                   + "\t" + plusIndent("seqz") + reg1 + ", " + reg1 + "\n";
                    break;
                case '>':
                    if (rightTmp->opType.length() == 1) // ">"
                        codeRiscV += "\t" + plusIndent("sgt") + regComb;
                    else // ">="
                        codeRiscV += "\t" + plusIndent("slt") + regComb
                                   + "\t" + plusIndent("seqz") + reg1 + ", " + reg1 + "\n";
                    break;
                case '&':
                    codeRiscV += "\t" + plusIndent("snez") + reg1 + ", " + reg2 + "\n"
                               + "\t" + plusIndent("snez") + regCallee[0] + ", " + reg3 + "\n"
                               + "\t" + plusIndent("and") + reg1 + ", " + reg1 + ", " + regCallee[0] + "\n";
                    break;
                case '|':
                    codeRiscV += "\t" + plusIndent("or") + regComb
                               + "\t" + plusIndent("snez") + reg1 + ", " + reg1 + "\n";
                    break;
                case '!':
                    codeRiscV += "\t" + plusIndent("xor") + regComb
                               + "\t" + plusIndent("snez") + reg1 + ", " + reg1 + "\n";
                    break;
                case '=':
                    codeRiscV += "\t" + plusIndent("xor") + regComb
                               + "\t" + plusIndent("seqz") + reg1 + ", " + reg1 + "\n";
                    break;
                default:
                    break;
            }
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

        // tigger
        codeTigger += "\tloadaddr " + tv->nameTigger + " " + reg1 + "\n";
        // riscv
        ifSW = false;
        codeRiscV += "\t" + plusIndent("la") + reg1 + ", " + tv->nameTigger + "\n";
    }
    else {
        EntryVarEE *tv = entryCurFunc->tmpVars[symbol->name];
        if (tv->ifArray) {
            // tigger
            codeTigger += "\tloadaddr " + to_string(tv->addr) + " " + reg1 + "\n";
            // riscv
            genADDI(reg1, "sp", tv->addr * 4);
        }
        else {
            // tigger
            codeTigger += "\tload " + to_string(tv->addr) + " " + reg1 + "\n";
            // riscv
            genLW(reg1, "sp", tv->addr * 4);
        }
    }

    storeIntoReg(rt2, array->right);

    // tigger
    codeTigger += "\t" + reg1 + " = " + reg1 + " + " + reg2 + "\n";
    // riscv
    ifSW = false;
    codeRiscV += "\t" + plusIndent("add") + reg1 + ", " + reg1 + ", " + reg2 + "\n";
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

        // tigger
        codeTigger += "\tloadaddr " + tv->nameTigger + " t6\n"
                    + "\tt6[0] = " + reg + "\n";
        // riscv
        ifSW = false;
        codeRiscV += "\t" + plusIndent("la") + regCaller[6] + ", " + tv->nameTigger + "\n";
        genSW(reg, regCaller[6], 0);
    }
    else {
        EntryVarEE *tv = entryCurFunc->tmpVars[valTmp->name];

        // tigger
        codeTigger +="\tstore " + reg + " " + to_string(tv->addr) + "\n";
        // riscv
        genSW(reg, "sp", tv->addr * 4);
    }
    return;
}