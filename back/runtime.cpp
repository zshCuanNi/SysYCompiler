#include "runtime.hpp"

Register::Register(RegType _regType)
    : regType(_regType), name(regName[(int)regType]) {}

StackElement::StackElement(int _offset,
                           EntryVarEE *_var,
                           bool _ifUpdated)
    : offset(_offset), var(_var), ifUpdated(_ifUpdated) {}

RuntimeEnv::RuntimeEnv() {
    for (int i = 0; i < 28; i++)
        regs[i] = Register((RegType)i);
}

// void RuntimeEnv::useReg(int regNo, const string &val) {
//     // update to-be-use register
//     regs[regNo].vals.clear();
//     regs[regNo].vals.push_back(val);
//     regs[regNo].ifVal = false;
//     // remove other variables' register descriptor
//     for (auto &i : tmpVars) {
//         for (auto it = i.second->regs.begin(); i != i.second->regs.end();)
//             if (*it == regNo) 
//                 it = i.second->regs.erase(it);
//             else
//                 it++;
//     }
//     tmpVars[val]->regs.push_back(regNo);
// }

// void RuntimeEnv::refresh() {
//     // only refresh stack elements which store single values
//     // because arrays are always updated
//     for (auto &i : stack)
//         if (!i.var->ifArray)
//             i.ifUpdated = false;
//     for (Register &i : regs) {
//         i.vals.clear();
//         i.ifVal = false;
//     }
// }
