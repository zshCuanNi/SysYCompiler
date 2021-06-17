#ifndef RUNTIME_H
#define RUNTIME_H

#include "symtab_ee.hpp"

enum RegType {
    x0,
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,  s11,
    t0, t1, t2, t3, t4, t5, t6,
    a0, a1, a2, a3, a4, a5, a6, a7
};

static const string regName[] = { "x0", "s0", "s1", "s2", "s3", "s4", "s5",
    "s6", "s7", "s8", "s9", "s10", "s11", "t0", "t1", "t2", "t3",
    "t4", "t5", "t6", "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7" };

static const string regZero[] = { "x0" };
static const string regCallee[] = { 
    "s0", "s1", "s2", "s3", "s4", "s5",
    "s6", "s7", "s8", "s9", "s10", "s11"
};
static const string regCaller[] = {
    "t0", "t1", "t2", "t3",
    "t4", "t5", "t6"
};
static const string regReturn[] = {
    "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7"
};

static const int regZeroNo[] = { 0 };
static const int regCalleeNo[] = { 1,2,3,4,5,6,7,8,9,10,11,12 };
static const int regCallerNo[] = { 13,14,15,16,17,18,19 };
static const int regReturnNo[] = { 20,21,22,23,24,25,26,27 };

class Register {
public:
    RegType regType;
    string name;
    bool ifVal = false;
    int value;
    vector<string> vals;

    Register() = default;
    Register(RegType _regType);
};

class StackElement {
public:
    EntryVarEE *var;// nullptr if it stores a register's value
    int offset;     // after / SIZE_OF_INT
    bool ifUpdated; // for the element reserved for callee saved registers
                    // ifUpdated is true if current value of the register is not the original value saved in the element
                    // ifUpdated is always true for arrays
                    // because arrays are always updated
    StackElement() = default;
    StackElement(int _offset, EntryVarEE *_var = nullptr, bool _ifUpdated = false);
};

class RuntimeEnv {
public:
    vector<StackElement> stack;
    Register regs[28];
    // unordered_map<string, int> stackPos;    // record where the value is stored in the stack
                                            // for an array, it records the first address

    RuntimeEnv();
    // void useReg(int regNo, const string &val);
    // void refresh();
};

#endif